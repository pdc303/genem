#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "m68000.h"
#include "m68000_times.c"
#include "types.h"
#include "memory.h"
#include "sysmem.h"
#include "debug.h"
#include "sysutil.h"
#include "bitman.h"
#include "world.h"
#include "thread.h"
#include "throttle.h"
#include "sysutil.h"

const char* opcode_names[256] =
{
	[OPCODE_BITMANIP] = "BITMANIP",
	[OPCODE_MOVEB] = "MOVE.B",
	[OPCODE_MOVEL] = "MOVE.L",
	[OPCODE_MOVEW] = "MOVE.W",
	[OPCODE_MOVEQ] = "MOVEQ",
	[OPCODE_MISC] = "MISC",
	[OPCODE_BRANCH] = "BRANCH",
	[OPCODE_DBCC] = "DBcc",
	[OPCODE_TST] = "TST",
	[OPCODE_LEA] = "LEA",
	[OPCODE_ANDI] = "ANDI",
	[OPCODE_ORI] = "ORI",
	[OPCODE_CMPI] = "CMPI",
	[OPCODE_EORI] = "EORI",
	[OPCODE_AND] = "AND",
	[OPCODE_OR] = "OR",
	[OPCODE_LSD] = "LSL/LSR",
	[OPCODE_ROD] = "ROL/ROR",
	[OPCODE_RTS] = "RTS",
	[OPCODE_CLR] = "CLR",
	[OPCODE_CMP] = "CMP",
	[OPCODE_CMPA] = "CMPA",
	[OPCODE_ADD] = "ADD",
	[OPCODE_ADDA] = "ADDA",
	[OPCODE_ADDQ] = "ADDQ",
	[OPCODE_SUBQ] = "SUBQ",
	[OPCODE_SUB] = "SUB",
	[OPCODE_SUBA] = "SUBA",
	[OPCODE_BTST] = "BTST",
	[OPCODE_NOP] = "NOP",
	[OPCODE_MOVEM] = "MOVEM",
	[OPCODE_MOVEUSP] = "MOVE USP",
};

int m68000_start(struct world *w)
{
	w->thread_m68k = genem_thread_create(m68000_main_thread, w);

	return 0;
}

void * m68000_main_thread(void *z)
{
	struct world *w = z;
	struct m68000 *m68k = PWORLD_PM68K(w);
	struct memory *mem = PWORLD_PMEM(w);
	struct throttle th;
	unsigned long target_usec;

	/* target microseconds per chunk of instructions */
	target_usec = (1.00 / M68000_HZ) * 1000000 * M68000_INSTRUCTIONS_CHUNK;

	throttle_init(&th, target_usec);

	while(!w->shutdown) {
		unsigned long usa, usb;
		double secs, mhz;
		usa = now_usec();
		m68000_expend_cycles(m68k, mem, M68000_INSTRUCTIONS_CHUNK);
		throttle(&th);
		usb = now_usec();
		secs = (usb - usa) / 1000000.0;
		mhz = (M68000_INSTRUCTIONS_CHUNK / secs) / 1000000.0;
	//	printf("Secs: %.2f\n", secs);
	//	printf("mhz: %.2f\n", mhz);
	}

	m68k = m68k;
	mem = mem;

	return NULL;
}

void m68000_expend_cycles(struct m68000 *m68k, struct memory *mem, gclock_t cycles)
{
	m68k->cycles = 0;

	while(m68k->cycles < cycles) {
		gclock_t ca, cb;
		ca = m68k->cycles;

		dbg_i("PC: 0x%X", (int) m68k->pc);

		m68000_exec(m68k, mem);
		
		cb = m68k->cycles;

		if(cb <= ca) {
			dbg_w("Instruction used no cycles");
		}
	}
}

int m68000_init(struct m68000 *m68k)
{
	memset(m68k, 0, sizeof(*m68k));
	m68000_init_register_pointers_array(m68k);
	m68k->a7 = SYSTEM_STACK_POS;

	return 0;
}

int m68000_init_register_pointers_array(struct m68000 *m68k)
{
	m68k->register_pointers[M68000_REGISTER_D0] = &m68k->d0;
	m68k->register_pointers[M68000_REGISTER_D1] = &m68k->d1;
	m68k->register_pointers[M68000_REGISTER_D2] = &m68k->d2;
	m68k->register_pointers[M68000_REGISTER_D3] = &m68k->d3;
	m68k->register_pointers[M68000_REGISTER_D4] = &m68k->d4;
	m68k->register_pointers[M68000_REGISTER_D5] = &m68k->d5;
	m68k->register_pointers[M68000_REGISTER_D6] = &m68k->d6;
	m68k->register_pointers[M68000_REGISTER_D7] = &m68k->d7;
	
	m68k->register_pointers[M68000_REGISTER_A0] = &m68k->a0;
	m68k->register_pointers[M68000_REGISTER_A1] = &m68k->a1;
	m68k->register_pointers[M68000_REGISTER_A2] = &m68k->a2;
	m68k->register_pointers[M68000_REGISTER_A3] = &m68k->a3;
	m68k->register_pointers[M68000_REGISTER_A4] = &m68k->a4;
	m68k->register_pointers[M68000_REGISTER_A5] = &m68k->a5;
	m68k->register_pointers[M68000_REGISTER_A6] = &m68k->a6;
	m68k->register_pointers[M68000_REGISTER_A7] = &m68k->a7;
	
	m68k->register_pointers[M68000_REGISTER_FP0] = &m68k->fp0;
	m68k->register_pointers[M68000_REGISTER_FP1] = &m68k->fp1;
	m68k->register_pointers[M68000_REGISTER_FP2] = &m68k->fp2;
	m68k->register_pointers[M68000_REGISTER_FP3] = &m68k->fp3;
	m68k->register_pointers[M68000_REGISTER_FP4] = &m68k->fp4;
	m68k->register_pointers[M68000_REGISTER_FP5] = &m68k->fp5;
	m68k->register_pointers[M68000_REGISTER_FP6] = &m68k->fp6;
	m68k->register_pointers[M68000_REGISTER_FP7] = &m68k->fp7;

	return 0;
}

int m68000_exec(struct m68000 *m68k, struct memory *mem)
{
	gword inst;
	int result;
	int opcode;

	PM68000_GET_NEXT_INSTRUCTION(m68k, &inst);

	opcode = m68000_decode_opcode(inst);

	dbg_i("%s", opcode_names[opcode]);

	switch(opcode) {
		case OPCODE_ANDI:
		case OPCODE_ORI:
			result = m68000_exec_andi_ori_flexible(m68k, mem,  inst, opcode);
			break;
		case OPCODE_CMPI:
			result = m68000_exec_cmpi(m68k, mem,  inst);
			break;
		case OPCODE_EORI:
			result = m68000_exec_eori(m68k, mem,  inst);
			break;
		case OPCODE_MOVEB:
			result = m68000_exec_move(m68k, mem,  inst, sizeof(byte));
			break;
		case OPCODE_MOVEL:
			result = m68000_exec_move(m68k, mem, inst, sizeof(glong));
			break;
		case OPCODE_MOVEW:
			result = m68000_exec_move(m68k, mem, inst, sizeof(gword));
			break;
		case OPCODE_MOVEQ:
			result = m68000_exec_moveq(m68k, inst);
			break;
		case OPCODE_TST:
			result = m68000_exec_tst(m68k, mem, inst);
			break;
		case OPCODE_LEA:
			result = m68000_exec_lea(m68k, mem, inst);
			break;
		case OPCODE_BRANCH:
			result = m68000_exec_branch(m68k, mem, inst);
			break;
		case OPCODE_DBCC:
			result = m68000_exec_dbcc(m68k, mem, inst);
			break;
		case OPCODE_AND:
			result = m68000_exec_and(m68k, mem, inst);
			break;
		case OPCODE_OR:
			result = m68000_exec_or(m68k, mem, inst);
			break;
		case OPCODE_LSD:
			result = m68000_exec_lsd(m68k, mem, inst);
			break;
		case OPCODE_ROD:
			result = m68000_exec_rod(m68k, mem, inst);
			break;
		case OPCODE_RTS:
			result = m68000_exec_rts(m68k, mem);
			break;
		case OPCODE_CLR:
			result = m68000_exec_clr(m68k, mem, inst);
			break;
		case OPCODE_CMP:
		case OPCODE_CMPA:
			result = m68000_exec_cmp_flexible(m68k, mem,  inst, opcode);
			break;
		case OPCODE_ADD:
		case OPCODE_ADDA:
		case OPCODE_SUB:
		case OPCODE_SUBA:
			result = m68000_exec_add_sub_flexible(m68k, mem,  inst, opcode);
			break;
		case OPCODE_ADDQ:
		case OPCODE_SUBQ:
			result = m68000_exec_addq_subq_flexible(m68k, mem,  inst, opcode);
			break;
		case OPCODE_BTST:
			result = m68000_exec_btst(m68k, mem,  inst);
			break;
		case OPCODE_NOP:
			result = m68000_exec_nop(m68k);
			break;
		case OPCODE_MOVEM:
			result = m68000_exec_movem(m68k, mem,  inst);
			break;
		case OPCODE_MOVEUSP:
			result = m68000_exec_moveusp(m68k, mem,  inst);
			break;
		case OPCODE_MULU:
		case OPCODE_ABCD:
		case OPCODE_MULS:
		case OPCODE_EXG:
		default:
			dbg_f("Unhandled opcode");
			result = -1;
			break;
	}

	return result;
}

int m68000_decode_opcode(gword inst)
{
	int opcode;

	opcode = INST_OPCODE(inst);

	switch(opcode) {
		case OPCODE_MOVEB:
		case OPCODE_MOVEL:
		case OPCODE_MOVEW:
		case OPCODE_BRANCH:
		case OPCODE_MOVEQ:
			return opcode;
		case OPCODE_MISC:
			if(INST_MISC_IS_TST(inst)) {
				return OPCODE_TST;
			} else if(INST_MISC_IS_LEA(inst)) {
				return OPCODE_LEA;
			} else if(INST_MISC_IS_RTS(inst)) {
				return OPCODE_RTS;
			} else if(INST_MISC_IS_CLR(inst)) {
				return OPCODE_CLR;
			} else if(INST_MISC_IS_NOP(inst)) {
				return OPCODE_NOP;
			} else if(INST_MISC_IS_MOVEM(inst)) {
				return OPCODE_MOVEM;
			} else if(INST_MISC_IS_MOVEUSP(inst)) {
				return OPCODE_MOVEUSP;
			}
			break;
		case OPCODE_BITMANIP:
			if(INST_BITMANIP_IS_ANDI(inst)) {
				return OPCODE_ANDI;
			} else if(INST_BITMANIP_IS_CMPI(inst)) {
				return OPCODE_CMPI;
			} else if(INST_BITMANIP_IS_EORI(inst)) {
				return OPCODE_EORI;
			} else if(INST_BITMANIP_IS_BTST(inst)) {
				return OPCODE_BTST;
			} else if(INST_BITMANIP_IS_ORI(inst)) {
				return OPCODE_ORI;
			}
			break;
		case OPCODE_ANDMUL:
			if(INST_ANDMUL_IS_MULU(inst)) {
				return OPCODE_MULU;
			} else if(INST_ANDMUL_IS_ABCD(inst)) {
				return OPCODE_ABCD;
			} else if(INST_ANDMUL_IS_MULS(inst)) {
				return OPCODE_MULS;
			} else if(INST_ANDMUL_IS_AND(inst)) {
				return OPCODE_AND;
			} else if(INST_ANDMUL_IS_EXG(inst)) {
				return OPCODE_EXG;
			}
			break;
		case OPCODE_ORDIVSBCD:
			if(INST_ORDIVSBCD_IS_OR(inst)) {
				return OPCODE_OR;
			}
			break;
		case OPCODE_SHIFTROTBIT:
			if(INST_SHIFTROTBIT_IS_LSD(inst)) {
				return OPCODE_LSD;
			} else if(INST_SHIFTROTBIT_IS_ROD(inst)) {
				return OPCODE_ROD;
			}
			break;
		case OPCODE_ADDQSUBQ:
			if(INST_ADDQSUBQ_IS_DBCC(inst)) {
				return OPCODE_DBCC;
			}
			if(INST_ADDQSUBQ_IS_ADDQ(inst)) {
				return OPCODE_ADDQ;
			}
			if(INST_ADDQSUBQ_IS_SUBQ(inst)) {
				return OPCODE_SUBQ;
			}
			break;
		case OPCODE_CMPEOR:
			if(INST_CMPEOR_IS_CMP(inst)) {
				return OPCODE_CMP;
			}
			if(INST_CMPEOR_IS_CMPA(inst)) {
				return OPCODE_CMPA;
			}
			break;
		case OPCODE_ADDADDX:
			if(INST_ADDADDX_IS_ADDX(inst)) {
				dbg_f("");
			} else if(INST_ADDADDX_IS_ADD(inst)) {
				return OPCODE_ADD;
			} else if(INST_ADDADDX_IS_ADDA(inst)) {
				return OPCODE_ADDA;
			}
			break;
		case OPCODE_SUBSUBX:
			if(INST_SUBSUBX_IS_SUBX(inst)) {
				dbg_f("");
			} else if(INST_SUBSUBX_IS_SUB(inst)) {
				return OPCODE_SUB;
			} else if(INST_SUBSUBX_IS_SUBA(inst)) {
				return OPCODE_SUBA;
			}
			break;
	}
	
	dbg_f("Unrecognised opcode: %d", opcode);
	return -1;
}

int m68000_exec_moveq(struct m68000 *m68k, gword inst)
{
	byte data;
	enum M68000_REGISTER reg;

	data = WORD_BYTE(inst);
	reg = M68000_REGISTER_D0 + BITS(inst, 9, 11);

	m68000_register_set(m68k, reg, (glong) data, sizeof(glong));

	CC(BASE_TIME_MOVEQ);

	CCR_N_SETX(m68k->status, data < 0);
	CCR_Z_SETX(m68k->status, data == 0);
	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	return 0;
}

int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size)
{
	int result;
	struct operand_info src, dest;
	/* longest source value will be a long */
	glong source_val;

	src.size = dest.size = size;

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_SOURCE, &src);
	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src);
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_DEST, &dest);
	m68000_inst_set_operand_dest(m68k, mem, &dest, source_val);
	
	if(dest.type == OPERAND_TYPE_IMMEDIATE) {
		dbg_f("Immediate data destination invaid for MOVE");
		return -1;
	}

	/* set flags */
	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	if(source_val == 0) {
		CCR_Z_SET(m68k->status);
	} else {
		CCR_Z_UNSET(m68k->status);
	}
	
	if(source_val < 0) {
		CCR_N_SET(m68k->status);
	} else {
		CCR_N_UNSET(m68k->status);
	}

	CC(move_time[size == sizeof(glong) ? 1 : 0][src.mode][dest.mode]);

	return result;
}

int m68000_inst_get_operand_info(struct m68000 *m68k, struct memory *mem, gword inst,
					int operand_type, struct operand_info *oi)
{
	int ea_mode, ea_register;

	/* if we have been given a size, use it. otherwise we may need to find and return the size */

	/*

	if((size != NULL) && (*size != 0)) {
		local_size = *size;
	} else {
		local_size = m68000_inst_get_operand_size(m68k, mem, inst);
		
		if(size != NULL) {
			*size = local_size;
		}
	}
	*/

	if(oi->size == 0) {
		oi->size = m68000_inst_get_operand_size(m68k, mem, inst);
	}

	if(operand_type == OPERAND_SOURCE) {
		ea_mode = INST_SOURCE_EA_MODE(inst);
		ea_register = INST_SOURCE_EA_REGISTER(inst);
	} else {
		ea_mode = INST_DEST_EA_MODE(inst);
		ea_register = INST_DEST_EA_REGISTER(inst);
	}
	
	switch(ea_mode) {
		case EA_MODE_DATA_REGISTER_DIRECT:
				oi->type = OPERAND_TYPE_REGISTER;
				oi->mode = ADDRESSING_MODE_DN;
				oi->reg = M68000_REGISTER_D0 + ea_register;
			break;
		case EA_MODE_ADDRESS_REGISTER_DIRECT:
				oi->type = OPERAND_TYPE_REGISTER;
				oi->mode = ADDRESSING_MODE_AN;
				oi->reg = M68000_REGISTER_A0 + ea_register;
			break;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_PREDEC:
			PM68000_REG_OFF_DECX(m68k, a, ea_register, oi->size);
			oi->mode = ADDRESSING_MODE_AN_IND_PREDEC;
			/* now fall through to EA_MODE_ADDRESS_REGISTER_INDIRECT */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC:
			/* if POSTINC, do the normal INDIRECT action here then increment */
			/* ... fall through ... */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT:
			oi->type = OPERAND_TYPE_INDIRECT;
			oi->address = PM68000_REG_OFF_VAL(m68k, a, ea_register);
			if(ea_mode == EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC) {
				PM68000_REG_OFF_INCX(m68k, a, ea_register, oi->size);
				oi->mode = ADDRESSING_MODE_AN_IND_POSTINC;
			} else if(ea_mode == EA_MODE_ADDRESS_REGISTER_INDIRECT) {
				oi->mode = ADDRESSING_MODE_AN_IND;
			}
			break;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_DISPLACEMENT:
		{
			gword displacement;
			PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
			/* sign extension omitted. don't think I need it. */
			oi->type = OPERAND_TYPE_INDIRECT;
			oi->mode = ADDRESSING_MODE_AN_IND_DISPL;
			oi->address = PM68000_REG_OFF_VAL(m68k, a, ea_register) + displacement;
			break;
		}
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_INDEX:
			oi->type = OPERAND_TYPE_INDIRECT;
			oi->mode = ADDRESSING_MODE_AN_IND_INDEX;
			dbg_f("Source with Index not yet implemented");
			goto m68000_inst_get_source_info_error;
		case EA_MODE_NOREG:
			switch(ea_register) {
				case EA_REGISTER_PROGRAM_COUNTER_INDIRECT_DISPLACEMENT:
				{
					gword displacement, displacement_base;
					/*
					the displacement is relative to the address
					of the extension word so save this address now
					*/
					displacement_base = m68k->pc;

					PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
					oi->type = OPERAND_TYPE_INDIRECT;
					oi->mode = ADDRESSING_MODE_PC_IND_DISPL;
					oi->address = (size_t) (displacement_base + displacement);
					goto m68000_inst_get_source_info_out;
				}
				case EA_REGISTER_PROGRAM_COUNTER_INDIRECT_INDEX:
					dbg_f("Source with Index not yet implemented");
					oi->type = OPERAND_TYPE_INDIRECT;
					oi->mode = ADDRESSING_MODE_PC_IND_INDEX;
					goto m68000_inst_get_source_info_error;
				case EA_REGISTER_ABSOLUTE_SHORT_ADDRESS:
				{
					gword address;
					PM68000_GET_NEXT_INSTRUCTION(m68k, &address);
					oi->type = OPERAND_TYPE_INDIRECT;
					oi->mode = ADDRESSING_MODE_ABS_SHORT;
					oi->address = address;
					goto m68000_inst_get_source_info_out;
				}
				case EA_REGISTER_ABSOLUTE_LONG_ADDRESS:
				{
					gword address_high;
					gword address_low;
					glong address;
					PM68000_GET_NEXT_INSTRUCTION(m68k, &address_high);
					PM68000_GET_NEXT_INSTRUCTION(m68k, &address_low);
					address = (address_high << (sizeof(gword) * 8)) | (address_low);

					oi->type = OPERAND_TYPE_INDIRECT;
					oi->mode = ADDRESSING_MODE_ABS_LONG;
					oi->address = address;
					goto m68000_inst_get_source_info_out;
				}
				case EA_REGISTER_IMMEDIATE_DATA:
				{
					oi->type = OPERAND_TYPE_IMMEDIATE;
					oi->mode = ADDRESSING_MODE_IMMEDIATE;
					goto m68000_inst_get_source_info_out;
				}
			}
		default:
			dbg_e("EA mode not implemented");
			goto m68000_inst_get_source_info_error;
	}

m68000_inst_get_source_info_out:
	return 0;
m68000_inst_get_source_info_error:
	dbg_f("Unable to get operand info from instruction");
	return -1;
}

int m68000_inst_get_operand_source_val(struct m68000 *m68k, struct memory *mem,
					struct operand_info *oi)
{
	glong source_val;

	source_val = 0;
	
	switch(oi->type) {
		case OPERAND_TYPE_REGISTER:
			source_val = *(long *) m68k->register_pointers[oi->reg];
			break;
		case OPERAND_TYPE_INDIRECT:
			memory_request(mem, oi->address, &source_val, oi->size, 1, &m68k->cycles);
//			if(oi->mode == ADDRESSING_MODE_AN_IND_POSTINC) {
//				PM68000_REG_ENUM_INCX(m68k, oi->reg, sizeof(gword));
//			}
			break;
		case OPERAND_TYPE_IMMEDIATE:
			source_val = m68000_get_immediate_value(m68k, mem, oi->size);
			break;
		default:
			dbg_e("Unknown source operand type");
			return -1;
	}

	source_val = glong_crop(source_val, oi->size);

	//return decode_2c(source_val, oi->size);
	return source_val;
}

int m68000_inst_set_operand_dest(struct m68000 *m68k, struct memory *mem,
					struct operand_info *oi, glong val)
{	
	switch(oi->type) {
		case OPERAND_TYPE_REGISTER:
			//*(glong *) (m68k->register_pointers[oi->reg]) = val;
			m68000_register_set(m68k, oi->reg, val, oi->size);
			break;
		case OPERAND_TYPE_INDIRECT:
			memory_write_wrapper(mem, oi->address, val, oi->size, 1, &m68k->cycles);
			break;
		case OPERAND_TYPE_IMMEDIATE:
			dbg_e("Immediate data destination invaid");
			return -1;
			break;
		default:
			dbg_f("Unknown dest operand type");
			return -1;
	}

	return 0;
}

int m68000_inst_get_operand_size(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int opcode;
	int size, size_format;
	int bytes;
	/* is this operand an int or float type? */
	int is_float;

	size = -1;
	is_float = 0;

	opcode = m68000_decode_opcode(inst);

	switch(opcode) {
		/* opcodes which need no size information */
		case OPCODE_LEA:
		/*
		AND combines size field and operation type so it can
		get picked in the AND function
		*/
		case OPCODE_AND:
				return 0;
		/* end of opcodes with no size information needed */
		case OPCODE_MOVEB:
		case OPCODE_MOVEL:
		case OPCODE_MOVEW:
				size_format = 1;
				size = BITS(inst, 12, 13);
				break;
		case OPCODE_ANDI:
		case OPCODE_ORI:
		case OPCODE_CMPI:
		case OPCODE_CMP:
		case OPCODE_EORI:
		case OPCODE_TST:
		case OPCODE_LSD:
		case OPCODE_ROD:
		case OPCODE_CLR:
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_ADDQ:
		case OPCODE_SUBQ:
				size_format = 2;
				size = BITS(inst, 6, 7);
				break;
		case OPCODE_ADDA:
		case OPCODE_SUBA:
				size_format = 3;
				size = BITS(inst, 6, 8);
				break;
		case OPCODE_MOVEM:
				size_format = 4;
				size = BIT(inst, 6);
	}

	if(size != -1) {
		if(size_format == 1) {
			switch(size) {
				case 1:
					bytes = sizeof(byte);
					break;
				case 3:
					bytes = sizeof(gword);
					break;
				case 2:
					bytes = sizeof(glong);
					break;
				default:
					dbg_f("Bad size field value");
			}
		} else if(size_format == 2) {
			switch(size) {
				case 0:
					bytes = sizeof(byte);
					break;
				case 1:
					bytes = sizeof(gword);
					break;
				case 2:
					bytes = sizeof(glong);
					break;
				default:
					dbg_f("Bad size field value");
			}
		} else if(size_format == 3) {
			switch(size) {
				case 3:
					bytes = sizeof(gword);
					break;
				case 7:
					bytes = sizeof(glong);
					break;
				default:
					dbg_f("Bad size field value");
			}
		} else if(size_format == 4) {
			switch(size) {
				case 0:
					bytes = sizeof(gword);
					break;
				case 1:
					bytes = sizeof(glong);
					break;
				default:
					dbg_f("Bad size field value");
			}
		}
	} else {
		dbg_f("No size bits set");
	}

//m68000_inst_get_operand_size_out:
	return bytes;
}

int m68000_exec_tst(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int result;
	struct operand_info src;
	int source_val;
	
	src.size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_SOURCE, &src);
	assert(result == 0);
	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src);

	/* validate size */
	
	switch(src.size) {
		case sizeof(byte):
		case sizeof(gword):
		case sizeof(glong):
			break;
		default:
			dbg_f("Bad SIZE field for TST instruction");
			result = -1;
			goto m68000_exec_tst_out;
	}

	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	if(source_val < 0) {
		CCR_N_SET(m68k->status);
	} else {
		CCR_N_UNSET(m68k->status);
	}

	if(source_val == 0) {
		CCR_Z_SET(m68k->status);
	} else {
		CCR_Z_UNSET(m68k->status);
	}

	CC(BASE_TIME_TST);
	if(src.type != OPERAND_TYPE_REGISTER) {
		CC(OI_TIME(src));
	}

m68000_exec_tst_out:
	return result;
}

int m68000_exec_branch(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int condition;
	glong displacement, displacement_base;
	int condition_true;
	int displacement_size;

	/*
	the displacement is relative to PC+2 (i.e. the word after
	the start of this instruction so let's save the PC now because
	our PC is actually PC+2 now as we just fetched the instruction.
	*/

	displacement_base = m68k->pc;
	
	condition = BITS(inst, 8, 11);
	condition_true = 0;

	/*
	we need to know the displacement size whether the branch is taken
	or not,	so look it up
	*/
		
	displacement = decode_2c(BITS(inst, 0, 7), sizeof(byte));
	if(displacement == 0x00) {
		displacement_size = sizeof(gword);
	} else if(displacement == 0xFF) {
		displacement_size = sizeof(glong);
	} else {
		displacement_size = sizeof(byte);
	}
	
	switch(condition) {
		case CONDITION_BSR:
			{
			glong future_pc;

			if(displacement_size > sizeof(byte)) {
				future_pc = m68k->pc + displacement_size;
			} else {
				future_pc = m68k->pc;
			}
			/*
			After a subroutine, execution should continue
			at the next instruction. Our pc will be at that location
			*/
			
			/* put the value on the stack */
			memory_write_wrapper(mem, m68k->a7, future_pc, sizeof(glong), 1, NULL);
			m68k->a7 -= sizeof(glong);
			/* fall through */
			}
		case CONDITION_BRA:
			condition_true = 1;
			break;
		default:
			condition_true = m68000_test_condition(m68k, condition);


			break;
	}

	
	if(condition_true) {
		/*
		if we need to branch, find the displacement, set the PC
		and set the timing
		*/	
		if(displacement_size == sizeof(gword)) {
			/* a 16-bit displacement follows the instruction */
			PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
			displacement = decode_2c(displacement, sizeof(gword));
		} else if(displacement_size == sizeof(glong)) {
			dbg_f("32-bit displacement for branch not supported on M68000");
			/* a 32-bit displacement follows the instruction */
			memory_request_easy(m68k->pc, glong, &displacement);
			/* bump pc two words */
			PM68000_PC_INCN(m68k, 2);
			displacement = decode_2c(displacement, sizeof(glong));
		}
		
		PM68000_PC_INCX(m68k, displacement - ((m68k->pc) - displacement_base));

		CC(BASE_TIME_BCC_Y);
	} else {
		CC(displacement_size == sizeof(byte) ?
						BASE_TIME_BCC_N_B:
						BASE_TIME_BCC_N_W);
	}

	return 0;
}

int m68000_exec_dbcc(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int condition;
	glong displacement, displacement_base;
	int condition_true;
	enum M68000_REGISTER reg;
	glong new_val;

	/*
	the displacement is relative to PC+2 (i.e. the word after
	the start of this instruction so let's save the PC now because
	our PC is actually PC+2 now as we just fetched the instruction.
	*/

	displacement_base = m68k->pc;
	/*
	initialise displacement because valgrind complained it never
	got initialised before being used, even though it did.
	*/
	displacement = 0;

	condition = BITS(inst, 8, 11);

	condition_true = m68000_test_condition(m68k, condition);

	if(condition_true) {
		CC(BASE_TIME_DBCC_N_CCTRUE);
		return 0;
	}

	/* decrement register value */

	reg = M68000_REGISTER_D0 + INST_SOURCE_EA_REGISTER(inst);

	m68000_register_set(m68k, reg, m68000_register_get(m68k, reg, sizeof(gword), 1) - 1,
						sizeof(gword));

	new_val = m68000_register_get(m68k, reg, sizeof(gword), 1);

	if(new_val == -1) {
		CC(BASE_TIME_DBCC_N_CCFALSE);
		/* bump PC to jump over the displacement word */
		PM68000_PC_INC(m68k);
		return 0;
	}

	/* if we reached this point, Dn!=-1 and CC is False so we are branching */
	/* find the displacement, set the PC and set the timing
	*/	
	/* a 16-bit displacement follows the instruction */
	
	PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
	displacement = decode_2c(displacement, sizeof(gword));
	PM68000_PC_INCX(m68k, displacement - ((m68k->pc) - displacement_base));
	CC(BASE_TIME_DBCC_Y_CCFALSE);
	return 0;
}


int m68000_test_condition(struct m68000 *m68k, int condition)
{
	int result;

	switch(condition) {
		case CONDITION_TRUE:
			result = 1;
			break;
		case CONDITION_FALSE:
			result = 0;
			break;
		case CONDITION_HI:
			result = !CCR_C(m68k->status) && !CCR_Z(m68k->status);
			break;
		case CONDITION_LS:
			result = CCR_C(m68k->status) || CCR_Z(m68k->status);
			break;
		case CONDITION_CC_HI:
			result = !CCR_C(m68k->status);
			break;
		case CONDITION_CC_LO:
			result = CCR_C(m68k->status);
			break;
		case CONDITION_NE:
			result = !CCR_Z(m68k->status);
			break;
		case CONDITION_EQ:
			result = CCR_Z(m68k->status);
			break;
		case CONDITION_VC:
			result = !CCR_V(m68k->status);
			break;
		case CONDITION_VS:
			result = CCR_V(m68k->status);
			break;
		case CONDITION_PL:
			result = !CCR_N(m68k->status);
			break;
		case CONDITION_MI:
			result = CCR_N(m68k->status);
			break;
		case CONDITION_GE:
			result = (CCR_N(m68k->status) && CCR_V(m68k->status)) ||
				((!CCR_N(m68k->status)) && (!CCR_V(m68k->status)));
			break;
		case CONDITION_LT:
			result = (CCR_N(m68k->status) && !CCR_V(m68k->status)) ||
				(!CCR_N(m68k->status) && CCR_V(m68k->status));
			break;
		case CONDITION_GT:
			result = (CCR_N(m68k->status) && CCR_V(m68k->status) &&
					!CCR_Z(m68k->status)) ||
				(!CCR_N(m68k->status) && !CCR_V(m68k->status) &&
					!CCR_Z(m68k->status));
			break;
		case CONDITION_LE:
			result = (CCR_Z(m68k->status)) ||
				(CCR_N(m68k->status) && CCR_V(m68k->status)) ||
				(!CCR_N(m68k->status) && CCR_V(m68k->status));
			break;
		default:
			dbg_e("Unsupported condition type");
	}

	return result;
}

/* get an immediate value which follows an instruction */
glong m68000_get_immediate_value(struct m68000 *m68k, struct memory *mem, int size)
{
	gword instx[2];
	glong val;

	PM68000_GET_NEXT_INSTRUCTION(m68k, &(instx[0]));

	switch(size) {
		case sizeof(byte):
			val = WORD_BYTE(instx[0]);
			break;
		case sizeof(gword):
			val = instx[0];
			break;
		case sizeof(glong):
			PM68000_GET_NEXT_INSTRUCTION(m68k, &(instx[1]));
			val = (instx[0] << (sizeof(gword) * 8)) | (instx[1]);
			break;
	}

	return val;
}

int m68000_exec_andi_ori_flexible(struct m68000 *m68k, struct memory *mem, gword inst, int opcode)
{
	int result;
	struct operand_info dest;
	glong source_val, dest_val;

	dest.size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &dest);
	assert(result == 0);

	dest_val = m68000_inst_get_operand_source_val(m68k, mem, &dest);
	
	source_val = m68000_get_immediate_value(m68k, mem, dest.size);

	switch(opcode) {
		case OPCODE_ANDI:
			dest_val &= source_val;
			break;
		case OPCODE_ORI:
			dest_val |= source_val;
			break;
	}


	m68000_inst_set_operand_dest(m68k, mem, &dest, dest_val);

	if(IS_NEG(dest_val, dest.size)) {
		CCR_N_SET(m68k->status);
	} else {
		CCR_N_UNSET(m68k->status);
	}

	if(dest_val == 0) {
		CCR_Z_SET(m68k->status);
	} else {
		CCR_Z_UNSET(m68k->status);
	}

	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	if(dest.type == OPERAND_TYPE_REGISTER) {
		CC(dest.size == sizeof(glong) ? BASE_TIME_ANDI_DN_L: BASE_TIME_ANDI_DN_BW);
	} else {
		CC(dest.size == sizeof(glong) ? BASE_TIME_ANDI_M_L: BASE_TIME_ANDI_M_BW);
		CC(OI_TIME(dest));
	}

	return result;
		
}

int m68000_exec_lea(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int result;
	struct operand_info ea;
	int regno;
	glong ea_val;

	ea.size = -1; /* do not look for a size */
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);
	assert(result == 0);

	/* get the Address register to store the result */
	regno = BITS(inst, 9, 11);

	switch(ea.type) {
		case OPERAND_TYPE_INDIRECT:
			ea_val = ea.address;
			break;
		case OPERAND_TYPE_REGISTER:
		case OPERAND_TYPE_IMMEDIATE:
		default:
			dbg_f("Invalid EA for LEA instruction (type %d)", ea.type);
			break;
	}
	
	PM68000_REG_OFF_SET(m68k, a, regno, ea_val);

	CC(lea_time[ea.mode]);

	return 0;
}

int m68000_exec_and(struct m68000 *m68k, struct memory *mem, gword inst)
{
	return m68000_exec_and_or(m68k, mem, inst, BITWISE_AND);
}
int m68000_exec_or(struct m68000 *m68k, struct memory *mem, gword inst)
{
	return m68000_exec_and_or(m68k, mem, inst, BITWISE_OR);
}

/*
The AND and OR operations are the same apart from the bitwise operation being performed
so we will have a common function for both
*/
int m68000_exec_and_or(struct m68000 *m68k, struct memory *mem, gword inst,
				enum BITWISE_OPERATION bop)
{
	int result;
	struct operand_info ea;
	int regno, opmode;
	int operation_mode;
	glong operands[2];
	glong and_result;

	regno = BITS(inst, 9, 11);
	opmode = BITS(inst, 6, 8);

	//operation_mode = BITS_3(opmode) >> 2;
	operation_mode = BIT(opmode, 3);

	switch(opmode & 3) {
		case 0:
			ea.size = sizeof(byte);
			break;
		case 1:
			ea.size = sizeof(gword);
			break;
		case 2:
			ea.size = sizeof(glong);
			break;
		default:
			dbg_f("Invalid size for operation (%d)", opmode & 3);
			break;
	}

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);
	assert(result == 0);

	/* get value from given register */
	operands[0] = PM68000_REG_OFF_VAL(m68k, d, regno);

	/* get value from given EA */
	operands[1] = m68000_inst_get_operand_source_val(m68k, mem, &ea);

	switch(bop) {
		case BITWISE_AND:
			and_result = operands[0] & operands[1];
			break;
		case BITWISE_OR:
			and_result = operands[0] | operands[1];
			break;
	}

	glong_crop(and_result, ea.size);

	if(operation_mode == EA_TO_REG) {
		m68000_register_set(m68k, M68000_REGISTER_D0 + regno,
					and_result, ea.size);
		CC(ea.size == sizeof(glong) ?
				BASE_TIME_AND_EA_TO_REG_L:
				BASE_TIME_AND_EA_TO_REG_BW);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, and_result);
		CC(ea.size == sizeof(glong) ?
				BASE_TIME_AND_REG_TO_EA_L:
				BASE_TIME_AND_REG_TO_EA_BW);
	}
	
	if(IS_NEG(and_result, ea.size)) {
		CCR_N_SET(m68k->status);
	} else {
		CCR_N_UNSET(m68k->status);
	}

	if(and_result == 0) {
		CCR_Z_SET(m68k->status);
	} else {
		CCR_Z_UNSET(m68k->status);
	}

	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	return 0;
}

void m68000_register_set(struct m68000 *m68k, enum M68000_REGISTER reg, glong val, int size)
{
	/*
	Relying on the fact this parameter is glong to take care of
	sign extentension!
	*/

	if((reg >= M68000_REGISTER_D0) && (reg <= M68000_REGISTER_D7)) {
		/*
		when writing to a data register, the bits affected should
		only be equal to the operand size
		*/

		val = encode_2c(val, size);

		switch(size) {
			case sizeof(byte):
				/* clear the lower byte */
				*(glong *)(m68k->register_pointers[reg]) &= ~0xFF;
				break;
			case sizeof(gword):
				/* clear the lower word */
				*(glong *)(m68k->register_pointers[reg]) &= ~0xFFFF;
				break;
			case sizeof(glong):
				*(glong *)(m68k->register_pointers[reg]) = 0;
				break;
		}

		/* merge the value */
		*(glong *)(m68k->register_pointers[reg]) |= val;

	} else if((reg >= M68000_REGISTER_A0) && (reg <= M68000_REGISTER_A7)) {
		/*
		when writing to an address register, the full 32 bits
		should be set
		*/
		*(glong *)(m68k->register_pointers[reg]) = val;
	}
}

glong m68000_register_get(struct m68000 *m68k, enum M68000_REGISTER reg, int size, int decode)
{
	glong result;

	if(
	((reg >= M68000_REGISTER_D0) && (reg <= M68000_REGISTER_D7))||
	((reg >= M68000_REGISTER_A0) && (reg <= M68000_REGISTER_A7))
	) {
		result = *(glong *)(m68k->register_pointers[reg]);
	}

	switch(size) {
		case sizeof(glong):
		case sizeof(gword):
		case sizeof(byte):
			result = glong_crop(result, size);
			break;
		default:
			dbg_f("Unhandled size (%d)", size);
			break;
	}

	if(decode) {
		result = decode_2c(result, size);
	}

	return result;
}

int m68000_exec_lsd(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int ir, dr, cr, shift_count;
	enum M68000_REGISTER reg;
	int shift_type;
	struct operand_info ea;
	int result;
	glong to_shift;

#define LSD_REGISTER_SHIFT 0
#define LSD_MEMORY_SHIFT 1

	result = 0;
	
	dr = BIT(inst, 8);

	if(BITS(inst, 6, 7) == 0x3) {
		shift_type = LSD_MEMORY_SHIFT;
	} else {
		shift_type = LSD_REGISTER_SHIFT;
	}

	if(shift_type == LSD_REGISTER_SHIFT) {

		ir = BIT(inst, 5);
		ea.size = m68000_inst_get_operand_size(m68k, mem, inst);
		reg = M68000_REGISTER_D0 + INST_SOURCE_EA_REGISTER(inst);

		to_shift = m68000_register_get(m68k, reg, ea.size, 0);

		/*
		shift count: immediate or register?
		ir = 0: this field contains the shift count.
		ir = 1: shift count = register contents % 64.
		*/
		cr = BITS(inst, 9, 11);

		if(ir == 0) {
			shift_count = cr;
			if(cr == 0) {
				shift_count = 8;
			}
		} else {
			/*
			presuming the register value does not need 2c decoding as a negative
			value would not make sense.
			PRM says this is limited to 64 so modulo the value 64.
			*/
			shift_count = m68000_register_get(m68k, reg, ea.size, 0) % 64;
		}
	} else {
		shift_count = 1;
		ea.size = sizeof(gword);
		result = m68000_inst_get_operand_info(m68k, mem, inst,
						OPERAND_ONE, &ea);
		to_shift = m68000_inst_get_operand_source_val(m68k, mem, &ea);

	}

	if(dr == 0) {
		/* right shift */
		if(shift_count > 0) {
			/* shift all but one */
			to_shift >>= shift_count - 1;
			/* if shift_count>0, set to the last bit shifted out. otherwise unaffected */
			CCR_X_SETX(m68k->status, get_bit(to_shift, 0));
			/* if shift_count>0, set to the last bit shifted out. otherwise cleared */
			CCR_C_SETX(m68k->status, CCR_X(m68k->status));
			/* shift out the last bit */
			to_shift >>= 1;
		} else {
			CCR_C_UNSET(m68k->status);
		}
	} else {
		/* left shift */
		if(shift_count > 0) {
			/* shift all but one */
			to_shift <<= shift_count - 1;
			/* if shift_count>0, set to the last bit shifted out. otherwise unaffected */
			CCR_X_SETX(m68k->status, get_bit(to_shift, 0));
			/* if shift_count>0, set to the last bit shifted out. otherwise cleared */
			CCR_C_SETX(m68k->status, CCR_X(m68k->status));
			/* shift out the last bit */
			to_shift <<= 1;
		}
	}

	CCR_N_SETX(m68k->status, (to_shift < 0));
	CCR_Z_SETX(m68k->status, (to_shift == 0));
	CCR_V_UNSET(m68k->status);

	/* now to put the shifted value in the dest */
	
	if(shift_type == LSD_REGISTER_SHIFT) {
		m68000_register_set(m68k, reg, to_shift, ea.size);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, to_shift);
	}
	CC(BASE_TIME_LSD_MEM);
	if(shift_type == LSD_REGISTER_SHIFT) {
		CC(ea.size == sizeof(glong) ? 
			BASE_TIME_LSD_REG_L : BASE_TIME_LSD_REG_BW);
		CC(2 * shift_count);
	}

	return 0;
}

int m68000_exec_rod(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int ir, dr, cr, shift_count;
	enum M68000_REGISTER reg;
	int shift_type;
	struct operand_info ea;
	int result;
	glong to_shift;
	enum LR direction;

	result = 0;
	
	dr = BIT(inst, 8);

	direction = dr ? LEFT : RIGHT;

	if(BITS(inst, 6, 7) == 0x3) {
		shift_type = LSD_MEMORY_SHIFT;
	} else {
		shift_type = LSD_REGISTER_SHIFT;
	}

	if(shift_type == LSD_REGISTER_SHIFT) {

		ir = BIT(inst, 5);
		ea.size = m68000_inst_get_operand_size(m68k, mem, inst);
		reg = M68000_REGISTER_D0 + INST_SOURCE_EA_REGISTER(inst);

		to_shift = m68000_register_get(m68k, reg, ea.size, 0);

		/*
		shift count: immediate or register?
		ir = 0: this field contains the shift count.
		ir = 1: shift count = register contents % 64.
		*/
		cr = BITS(inst, 9, 11);

		if(ir == 0) {
			shift_count = cr;
			if(cr == 0) {
				shift_count = 8;
			}
		} else {
			/*
			presuming the register value does not need 2c decoding as a negative
			value would not make sense.
			PRM says this is limited to 64 so modulo the value 64.
			*/
			shift_count = m68000_register_get(m68k, reg, ea.size, 0) % 64;
		}
	} else {
		shift_count = 1;
		ea.size = sizeof(gword);
		result = m68000_inst_get_operand_info(m68k, mem, inst,
						OPERAND_ONE, &ea);
		to_shift = m68000_inst_get_operand_source_val(m68k, mem, &ea);

	}

	/* rotate bits (not rotating the MSB) */
	to_shift = rotate_bits(to_shift, (ea.size * 8) - 2, 0, shift_count - 1, direction);
	/* grab the last rotated-out bit */
	CCR_C_SETX(m68k->status, direction == LEFT ?
				get_bit(to_shift, (ea.size * 8) - 2) :
				BIT(to_shift, 0))
	/* finish the rotate */
	to_shift = rotate_bits(to_shift, (ea.size * 8) - 2, 0, 1, direction);

	CCR_N_SETX(m68k->status, IS_NEG(to_shift, ea.size));
	CCR_Z_SETX(m68k->status, (to_shift == 0));
	CCR_V_UNSET(m68k->status);

	/* now to put the shifted value in the dest */
	if(shift_type == LSD_REGISTER_SHIFT) {
		m68000_register_set(m68k, reg, to_shift, ea.size);
		CC(ea.size == sizeof(glong) ? 
			BASE_TIME_ROD_REG_L : BASE_TIME_ROD_REG_BW);
		CC(2 * shift_count);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, to_shift);
		CC(BASE_TIME_ROD_MEM);
	}

	return 0;
}

int m68000_exec_rts(struct m68000 *m68k, struct memory *mem)
{
	glong popped;

#if 0
	popped = stack_pop(m68k->system_stack);

	m68000_register_set(m68k, M68000_REGISTER_A7,
			m68k->system_stack->stack_pointer, sizeof(glong));
	m68k->pc = popped;
#endif
	m68k->a7 += sizeof(glong);
	memory_request(mem, m68k->a7, &popped, sizeof(glong), 1, NULL);
	m68k->pc = popped;
	
	CC(BASE_TIME_RTS);

	return 0;
}

int m68000_exec_clr(struct m68000 *m68k, struct memory *mem, gword inst)
{
	struct operand_info ea;
	int result;

	ea.size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);
	assert(result == 0);

	m68000_inst_set_operand_dest(m68k, mem, &ea, 0);

	CCR_N_UNSET(m68k->status);
	CCR_Z_SET(m68k->status);
	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	if(ea.type == OPERAND_TYPE_REGISTER) {
		CC(ea.size == sizeof(glong) ? BASE_TIME_CLR_REG_L : BASE_TIME_CLR_REG_BW);
	} else {
		CC(ea.size == sizeof(glong) ? BASE_TIME_CLR_MEM_L : BASE_TIME_CLR_MEM_BW);
	}

	return 0;
}

int m68000_exec_cmp_flexible(struct m68000 *m68k, struct memory *mem, gword inst, int opcode)
{
	struct operand_info src;
	enum M68000_REGISTER dest_reg; /* dest data reg */
	glong dest_val;
	glong source_val;
	int r;
	
	/*
	ignoring the opmode field and letting the m68000_inst_get_operand_info
	get the size for us
	*/
	src.size = 0;
	r = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &src);
	assert(r == 0);

	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src);
	
	if(opcode == OPCODE_CMP) {
		/* CMP uses Dn */
		dest_reg = M68000_REGISTER_D0 + BITS(inst, 9, 11);
	} else {
		/* CMPA uses An */
		dest_reg = M68000_REGISTER_D0 + BITS(inst, 9, 11);
	}

	dest_val = m68000_register_get(m68k, dest_reg, src.size, 1);
	
	m68000_cmp_generalised(m68k, source_val, dest_val);

	CC(src.size == sizeof(glong) ? BASE_TIME_CMP_L : BASE_TIME_CMP_BW);

	return 0;
}

int m68000_exec_cmpi(struct m68000 *m68k, struct memory *mem, gword inst)
{
	struct operand_info dest;
	int result;
	glong dest_val, source_val;

	dest.size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &dest);
	assert(result == 0);
	dest_val = m68000_inst_get_operand_source_val(m68k, mem, &dest);
	source_val = m68000_get_immediate_value(m68k, mem, dest.size);

	m68000_cmp_generalised(m68k, source_val, dest_val);
	
	if(dest.type == OPERAND_TYPE_REGISTER) {
		CC(dest.size == sizeof(glong) ? BASE_TIME_CMPI_DN_L: BASE_TIME_CMPI_DN_BW);
	} else {
		CC(dest.size == sizeof(glong) ? BASE_TIME_CMPI_M_L: BASE_TIME_CMPI_M_BW);
		CC(OI_TIME(dest));
	}

	return 0;
}

/* generalised CMP function. expected sign extended values */
int m68000_cmp_generalised(struct m68000 *m68k, glong src, glong dest)
{
	glong subval;

	subval = dest - src;

	CCR_N_SETX(m68k->status, subval < 0);
	CCR_Z_SETX(m68k->status, subval == 0);
	/* do we have overflow? */
	CCR_V_SETX(m68k->status, (subval > dest) != (src > 0));
	/*
	Was there borrow by the MSB?
	Note this test is probably wrong or naive.
	*/
	CCR_C_SETX(m68k->status, src > dest);

	return 0;
}

glong m68000_eor_generalised(struct m68000 *m68k, glong src, glong dest, int size)
{
	glong out;

	out = src ^ dest;

	CCR_N_SETX(m68k->status, IS_NEG(out, size));
	CCR_Z_SETX(m68k->status, out == 0);
	CCR_V_UNSET(m68k->status);
	CCR_C_UNSET(m68k->status);

	return 0;
}

int m68000_exec_eori(struct m68000 *m68k, struct memory *mem, gword inst)
{
	struct operand_info dest;
	int result;
	glong dest_val, source_val, n;

	dest.size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &dest);
	assert(result == 0);
	dest_val = m68000_inst_get_operand_source_val(m68k, mem, &dest);
	source_val = m68000_get_immediate_value(m68k, mem, dest.size);

	n = m68000_eor_generalised(m68k, source_val, dest_val, dest.size);

	m68000_inst_set_operand_dest(m68k, mem, &dest, n);
	
	if(dest.type == OPERAND_TYPE_REGISTER) {
		CC(dest.size == sizeof(glong) ? BASE_TIME_EORI_DN_L: BASE_TIME_EORI_DN_BW);
	} else {
		CC(dest.size == sizeof(glong) ? BASE_TIME_EORI_M_L: BASE_TIME_EORI_M_BW);
		CC(OI_TIME(dest));
	}

	return 0;
}

int m68000_exec_add_sub_flexible(struct m68000 *m68k, struct memory *mem,
						gword inst, int opcode)
{
	enum M68000_REGISTER reg;
	struct operand_info ea;
	int r;
	glong source_val, dest_val, val;
	int opmode, operation_mode;

	opmode = BITS(inst, 6, 8);

	if((opcode == OPCODE_ADD) || (opcode == OPCODE_SUB)) {
		if(BIT(opmode, 3)) {
			operation_mode = REG_TO_EA;
			/* now we have noted the mode, fudge the instruction */
			/* make the size field to the expected format */
			inst = unset_bit(inst, 8);
		} else {
			operation_mode = EA_TO_REG;
		}
	} else if((opcode == OPCODE_ADDA) || (opcode == OPCODE_SUBA)) {
		operation_mode = EA_TO_REG;
	}

	ea.size = 0;
	r = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);
	assert(r == 0);

	reg = BITS(inst, 9, 11);
	switch(opcode) {
		case OPCODE_ADD:
		case OPCODE_SUB:
			reg += M68000_REGISTER_D0;
			break;
		case OPCODE_SUBA:
			reg += M68000_REGISTER_A0;
			break;
	}

	if(operation_mode == EA_TO_REG) {
		dest_val = m68000_register_get(m68k, reg, ea.size, 1);
		source_val = m68000_inst_get_operand_source_val(m68k, mem, &ea);
	} else { /* REG_TO_EA */
		source_val = m68000_register_get(m68k, reg, ea.size, 1);
		dest_val = m68000_inst_get_operand_source_val(m68k, mem, &ea);
	}
	
	switch(REAL_OPCODE(opcode)) {
		case OPCODE_ADDADDX:
			val = m68000_add_sub_generalised(m68k, source_val, dest_val,
							ea.size, OPCODE_ADD, 1);
			break;
		case OPCODE_SUBSUBX:
			val = m68000_add_sub_generalised(m68k, source_val, dest_val,
							ea.size, OPCODE_SUB, 1);
			break;
	}

	if(operation_mode == EA_TO_REG) {
		m68000_register_set(m68k, reg, val, ea.size);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, val);
	}

	/* timing for SUB is the same as ADD */

	if(opcode == OPCODE_SUB) {
		opcode = OPCODE_ADD;
	}

	if((opcode == OPCODE_ADD) || (opcode == OPCODE_ADDA)) {
		if(operation_mode == REG_TO_EA) {
			/* only OPCODE_ADD applicable here */
			CC(ea.size == sizeof(glong) ? BASE_TIME_ADD_REG_TO_EA_L:
						BASE_TIME_ADD_REG_TO_EA_BW);
		} else { /* EA_TO_REG */
			if(opcode == OPCODE_ADD) {
				CC(ea.size == sizeof(glong) ? BASE_TIME_ADD_EA_TO_REG_L:
							BASE_TIME_ADD_EA_TO_REG_BW);
			} else if(opcode == OPCODE_ADDA) {
				CC(ea.size == sizeof(glong) ? BASE_TIME_ADDA_EA_TO_REG_L:
							BASE_TIME_ADDA_EA_TO_REG_BW);
			}
			if((ea.type == OPERAND_TYPE_REGISTER) || 
				(ea.type == OPERAND_TYPE_IMMEDIATE)) {
				CC(2);
			}
		}
		CC(OI_TIME(ea));
	}

	return 0;
}

/* expects opcode as OPCODE_ADD or OPCODE_SUB to decide whether to add or subtract */
glong m68000_add_sub_generalised(struct m68000 *m68k, glong source, glong dest,
					int size, int opcode, int affect_ccr)
{
	glong gresult;

	/*
	here we use the correct integer widths so overflow and wraparound is
	done automatically
	*/
	if(size == sizeof(byte)) {
		byte result;
		if(opcode == OPCODE_ADD) {
			result = dest + source;
		} else {
			result = dest - source;
		}
		gresult = result;
	} else if(size == sizeof(gword)) {
		gword result;
		if(opcode == OPCODE_ADD) {
			result = dest + source;
		} else {
			result = dest - source;
		}
		gresult = result;
	} else if(size == sizeof(glong)) {
		glong result;
		if(opcode == OPCODE_ADD) {
			result = dest + source;
		} else {
			result = dest - source;
		}
		gresult = result;
	}

	if(affect_ccr) {
		CCR_N_SETX(m68k->status, gresult < 0);
		CCR_Z_SETX(m68k->status, gresult == 0);
		/* do we have overflow? */
		if(opcode == OPCODE_ADD) {
			CCR_V_SETX(m68k->status, (gresult < source) && (dest > 0));
		} else {
			CCR_V_SETX(m68k->status, (gresult > source) && (dest > 0));
		}
		/* XXX: Carry flag not set */
		CCR_C_UNSET(m68k->status);
		CCR_X_SETX(m68k->status, CCR_C(m68k->status));
	}

	return gresult;
}

int m68000_exec_addq_subq_flexible(struct m68000 *m68k, struct memory *mem, gword inst, int opcode)
{
	struct operand_info ea;
	int data_immediate;
	glong data_ea;
	glong result;
	int r;

	ea.size = 0;
	r = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);
	assert(r == 0);
	data_ea = m68000_inst_get_operand_source_val(m68k, mem, &ea);

	data_immediate = BITS(inst, 9, 11);
	if(data_immediate == 0) {
		data_immediate = 8;
	}
	
	result = m68000_add_sub_generalised(m68k, data_immediate, data_ea,
					ea.size, opcode == OPCODE_ADDQ ?
							OPCODE_ADD : OPCODE_SUB,
					ea.mode != ADDRESSING_MODE_AN);


	if(ea.type == OPERAND_TYPE_REGISTER) {
		CC(ea.size == sizeof(glong) ?
				BASE_TIME_ADDQ_SUBQ_TO_REG_BW :
				BASE_TIME_ADDQ_SUBQ_TO_REG_L);
	} else {
		CC(ea.size == sizeof(glong) ?
				BASE_TIME_ADDQ_SUBQ_TO_MEM_BW :
				BASE_TIME_ADDQ_SUBQ_TO_MEM_L);
		CC(OI_TIME(ea));
	}
	
	if(ea.mode == ADDRESSING_MODE_AN) {
		/*
		the PRM says that AN mode width WORD size affects the
		whole register. I am presuming this means we read the first
		16 bits but affect all 32 bits upon writing.
		So we will now say the size is LONG so that all bits are affected.
		*/
		ea.size = sizeof(glong);
	}

	m68000_inst_set_operand_dest(m68k, mem, &ea, result);

	return 0;
}

int m68000_exec_btst(struct m68000 *m68k, struct memory *mem, gword inst)
{
	struct operand_info dest_ea;
	glong bit_number, dest_val;
	
	/*
	do not automatically deduce the size. The size is complicated
	here because it depends on whether the destination is a register or memory.
	*/
	dest_ea.size = -1;
	m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &dest_ea);
	dest_ea.size = dest_ea.type == OPERAND_TYPE_REGISTER ? 
						sizeof(glong) :
						sizeof(byte);
	dest_val = m68000_inst_get_operand_source_val(m68k, mem, &dest_ea);

	if(BIT(inst, 8)) {
		/* get bit number from register */
		/* aka Dynamic bit number */
		enum M68000_REGISTER reg;

		reg = M68000_REGISTER_D0 + BITS(inst, 9, 11);
		/* bit number must be modulo'd depending on dest type */
		bit_number = m68000_register_get(m68k, reg, sizeof(glong), 0);
	} else {
		/* get bit number from memory */
		/* aka Static bit number */
		bit_number = m68000_get_immediate_value(m68k, mem, sizeof(byte));
	}

	if(dest_ea.type == OPERAND_TYPE_REGISTER) {
		bit_number %= 32;
		CC(BIT(inst, 8) ? BASE_TIME_BTST_DYNAMIC_REG_L : BASE_TIME_BTST_DYNAMIC_MEM_B);
	} else {
		bit_number %= 8;
		CC(BIT(inst, 8) ? BASE_TIME_BTST_STATIC_REG_L : BASE_TIME_BTST_STATIC_MEM_B);
		CC(OI_TIME(dest_ea));
	}

	CCR_Z_SETX(m68k->status, get_bit(dest_val, bit_number) == 0);

	return 0;
}

int m68000_exec_nop(struct m68000 *m68k)
{
	CC(BASE_TIME_NOP);
	return 0;
}

int m68000_exec_movem(struct m68000 *m68k, struct memory *mem, gword inst)
{
	int dr;
	int operation_mode;
	struct operand_info ea;
	gword register_mask;
	int i, action_count;

	dr = BIT(inst, 10);

	operation_mode = dr == 0 ? REG_TO_EA : EA_TO_REG;

	ea.size = 0;

	PM68000_GET_NEXT_INSTRUCTION(m68k, &register_mask);

	action_count = 0;

	for(i = 0; i < 16; i++) {
		enum M68000_REGISTER reg;

		if(!get_bit(register_mask, i))
			/* nothing to do for this reg */
			continue;

		action_count++;

		/* keep calling this so that the inc/decrement is re-performed */
		m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE, &ea);

		if(ea.mode == ADDRESSING_MODE_AN_IND_PREDEC) {
			reg = M68000_REGISTER_A7 - i;
		} else if(ea.mode == ADDRESSING_MODE_AN_IND_POSTINC) {
			reg = M68000_REGISTER_D0 + i;
		} else {
			dbg_f("EA not inc or dec. We currently don't handle this "
				"by automatically incrementing the address");
		}

		if(operation_mode == EA_TO_REG) {
			glong val;
			if(ea.size == sizeof(gword)) {
				/*
				word values from memory need to be sign-extended
				and put in to the register as a full LONG
				*/
				val = memory_request_gword(mem, ea.address, 1, NULL);
			} else {
				val = memory_request_glong(mem, ea.address, 1, NULL);
			}
		
			/* set the reg value */
			m68000_register_set(m68k, reg, val, ea.size);
		} else { /* REG_TO_EA */
			glong val;

			val = m68000_register_get(m68k, reg, ea.size, 1);
			m68000_inst_set_operand_dest(m68k, mem, &ea, val);
		}
	}
	
	if(operation_mode == EA_TO_REG) {
		CC(movem_mem_to_reg_time[ea.mode]);
	} else {
		CC(movem_reg_to_mem_time[ea.mode]);
	}

	CC((ea.size == sizeof(gword) ? 4 : 8) * action_count);
	
	return 0;
}

int m68000_exec_moveusp(struct m68000 *m68k, struct memory *mem, gword inst)
{
	enum M68000_REGISTER reg_src, reg_dst;

	if(BIT(inst, 3)) {
		/* USP->An */
		reg_dst = M68000_REGISTER_A0 + INST_SOURCE_EA_REGISTER(inst);
		reg_src = M68000_REGISTER_A7;
	} else {
		/* An->USP */
		reg_src = M68000_REGISTER_A0 + INST_SOURCE_EA_REGISTER(inst);
		reg_dst = M68000_REGISTER_A7;
	}
	
	m68000_register_set(m68k, reg_dst,
		m68000_register_get(m68k, reg_src, sizeof(glong), 1),
			sizeof(glong));

	CC(BASE_TIME_MOVEUSP);

	return 0;
}
