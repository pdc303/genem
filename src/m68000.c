#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "m68000.h"
#include "types.h"
#include "memory.h"
#include "sysmem.h"
#include "debug.h"
#include "sysutil.h"
#include "bitman.h"

const char* opcode_names[256] =
{
	[OPCODE_BITMANIP] = "BITMANIP",
	[OPCODE_MOVEB] = "MOVE.B",
	[OPCODE_MOVEL] = "MOVE.L",
	[OPCODE_MOVEW] = "MOVE.W",
	[OPCODE_MISC] = "MISC",
	[OPCODE_BRANCH] = "BRANCH",
	[OPCODE_TST] = "TST",
	[OPCODE_LEA] = "LEA",
	[OPCODE_ANDI] = "AND.I",
	[OPCODE_AND] = "AND",
	[OPCODE_LSD] = "LSL/LSR",
};

int m68000_init(struct m68000 *m68k)
{
	memset(m68k, 0, sizeof(*m68k));
	m68000_init_register_pointers_array(m68k);
	stack_init(&m68k->system_stack, SYSTEM_STACK_SIZE, sizeof(gword));

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

int m68000_exec(struct m68000 *m68k, struct memory *mem, int *cycles)
{
	gword inst;
	int result;
	int opcode;

	PM68000_GET_NEXT_INSTRUCTION(m68k, &inst);

	opcode = m68000_decode_opcode(inst);

	dbg_i("%s", opcode_names[opcode]);

	switch(opcode) {
		case OPCODE_ANDI:
			result = m68000_exec_andi(m68k, mem,  inst, cycles);
			break;
		case OPCODE_MOVEB:
			result = m68000_exec_move(m68k, mem,  inst, sizeof(byte), cycles);
			break;
		case OPCODE_MOVEL:
			result = m68000_exec_move(m68k, mem, inst, sizeof(glong), cycles);
			break;
		case OPCODE_MOVEW:
			result = m68000_exec_move(m68k, mem, inst, sizeof(gword), cycles);
			break;
		case OPCODE_TST:
			result = m68000_exec_tst(m68k, mem, inst, cycles);
			break;
		case OPCODE_LEA:
			result = m68000_exec_lea(m68k, mem, inst, cycles);
			break;
		case OPCODE_BRANCH:
			result = m68000_exec_branch(m68k, mem, inst, cycles);
			break;
		case OPCODE_AND:
			result = m68000_exec_and(m68k, mem, inst, cycles);
			break;
		case OPCODE_OR:
			result = m68000_exec_or(m68k, mem, inst, cycles);
			break;
		case OPCODE_LSD:
			result = m68000_exec_lsd(m68k, mem, inst, cycles);
			break;
		case OPCODE_MULU:
		case OPCODE_ABCD:
		case OPCODE_MULS:
		case OPCODE_EXG:
		default:
			dbg_e("Unhandled opcode: %d", opcode);
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
			return opcode;
		case OPCODE_MISC:
			if(INST_MISC_IS_TST(inst)) {
				return OPCODE_TST;
			} else if(INST_MISC_IS_LEA(inst)) {
				return OPCODE_LEA;
			}
			break;
		case OPCODE_BITMANIP:
			if(INST_BITMANIP_IS_ANDI(inst)) {
				return OPCODE_ANDI;
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
			}
	}
	
	dbg_f("Unrecognised opcode: %d", opcode);
	return -1;
}

int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size,
							int *cycles)
{
	int result;
	struct operand_info src, dest;
	/* longest source value will be a long */
	glong source_val;

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_SOURCE, &src, &size);
	assert(result == 0);
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_DEST, &dest, &size);
	assert(result == 0);
	
	switch(dest.type) {
		case OPERAND_TYPE_IMMEDIATE:
			dbg_f("Immediate data destination invaid for MOVE");
			break;
	}

	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src, size);
	m68000_inst_set_operand_dest(m68k, mem, &dest, source_val, size);
		
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

	return result;

//m68000_exec_move_out_error:
	return result;
}

int m68000_inst_get_operand_info(struct m68000 *m68k, struct memory *mem, gword inst,
					int operand_type, struct operand_info *oi, int *size)
{
	int ea_mode, ea_register;
	int local_size;

	memset(oi, 0, sizeof(*oi));

	/* if we have been given a size, use it. otherwise we may need to find and return the size */

	if((size != NULL) && (*size != 0)) {
		local_size = *size;
	} else {
		local_size = m68000_inst_get_operand_size(m68k, mem, inst);
		
		if(size != NULL) {
			*size = local_size;
		}
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
				oi->reg = M68000_REGISTER_D0 + ea_register;
			break;
		case EA_MODE_ADDRESS_REGISTER_DIRECT:
				oi->type = OPERAND_TYPE_REGISTER;
				oi->reg = M68000_REGISTER_A0 + ea_register;
			break;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_PREDEC:
			PM68000_REG_OFF_DECX(m68k, a, ea_register, local_size);
			/* now fall through to EA_MODE_ADDRESS_REGISTER_INDIRECT */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC:
			/* if POSTINC, do the normal INDIRECT action here then increment */
			/* ... fall through ... */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT:
			oi->type = OPERAND_TYPE_INDIRECT;
			oi->address = PM68000_REG_OFF_VAL(m68k, a, ea_register);
			if(ea_mode == EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC) {
				PM68000_REG_OFF_INCX(m68k, a, ea_register, local_size);
			}
			break;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_DISPLACEMENT:
		{
			gword displacement;
			PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
			/* sign extension omitted. don't think I need it. */
			oi->type = OPERAND_TYPE_INDIRECT;
			oi->address = PM68000_REG_OFF_VAL(m68k, a, ea_register) + displacement;
			break;
		}
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_INDEX:
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
					oi->address = (size_t) (displacement_base + displacement);
					goto m68000_inst_get_source_info_out;
				}
				case EA_REGISTER_PROGRAM_COUNTER_INDIRECT_INDEX:
					dbg_f("Source with Index not yet implemented");
					goto m68000_inst_get_source_info_error;
				case EA_REGISTER_ABSOLUTE_SHORT_ADDRESS:
				{
					gword address;
					PM68000_GET_NEXT_INSTRUCTION(m68k, &address);
					oi->type = OPERAND_TYPE_INDIRECT;
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
					oi->address = address;
					goto m68000_inst_get_source_info_out;
				}
				case EA_REGISTER_IMMEDIATE_DATA:
				{
					/* there will be a max of 6 extension words */
					gword ext[6];

					PM68000_GET_NEXT_INSTRUCTION(m68k, &(ext[0]));

					oi->type = OPERAND_TYPE_IMMEDIATE;
					switch(local_size) {
						case sizeof(byte):
							oi->data_int = WORD_BYTE(ext[0]);
							goto m68000_inst_get_source_info_out;
						case sizeof(gword):
							oi->data_int = ext[0];
							goto m68000_inst_get_source_info_out;
						case sizeof(glong):
							PM68000_GET_NEXT_INSTRUCTION(m68k, &(ext[1]));
							oi->data_int = (ext[0] << sizeof(gword))
									& ext[1];
							goto m68000_inst_get_source_info_out;
						default:
							dbg_e("Unsupported size for Source Immediate data");
							goto m68000_inst_get_source_info_error;
					}
					break;
				}
			}
			
		default:
			dbg_e("MOVE source mode not implemented");
			goto m68000_inst_get_source_info_error;
	}
m68000_inst_get_source_info_out:
	return 0;
m68000_inst_get_source_info_error:
	return -1;
}

int m68000_inst_get_operand_source_val(struct m68000 *m68k, struct memory *mem,
							struct operand_info *oi, int size)
{
	glong source_val;

	source_val = 0;
	
	switch(oi->type) {
		case OPERAND_TYPE_REGISTER:
			source_val = *(gword *) m68k->register_pointers[oi->reg];
			break;
		case OPERAND_TYPE_INDIRECT:
			memory_request(mem, oi->address, size, &source_val, 1);
			break;
		case OPERAND_TYPE_IMMEDIATE:
			source_val = oi->data_int;
			break;
		default:
			dbg_e("Unknown source operand type");
			return -1;
	}

	source_val = glong_crop(source_val, size);

	return decode_2c(source_val, size);
}

int m68000_inst_set_operand_dest(struct m68000 *m68k, struct memory *mem,
					struct operand_info *oi, glong val, int size)
{	
	switch(oi->type) {
		case OPERAND_TYPE_REGISTER:
			//*(glong *) (m68k->register_pointers[oi->reg]) = val;
			m68000_register_set(m68k, oi->reg, val, size);
			break;
		case OPERAND_TYPE_INDIRECT:
			dbg_e("memory_put... not implemented");
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
			size = BITS_12_13(inst);
			break;
		case OPCODE_ANDI:
		case OPCODE_TST:
		case OPCODE_LSD:
				size_format = 2;
				size = BITS_6_7(inst);
			break;
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

		}
	} else {
		dbg_f("No size bits set");
	}

//m68000_inst_get_operand_size_out:
	return bytes;
}

int m68000_exec_tst(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int size;
	int result;
	struct operand_info src;
	int source_val;

	size = 0;

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_SOURCE, &src, &size);
	assert(result == 0);
	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src, size);

	/* validate size */
	
	switch(size) {
		case sizeof(byte):
		case sizeof(gword):
		case sizeof(glong):
			break;
		default:
			dbg_e("Bad SIZE field for TST instruction");
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

m68000_exec_tst_out:
	return result;
}

int m68000_exec_branch(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int condition;
	glong displacement, displacement_base;
	int condition_true;

	condition = BITS_8_11(inst);
	displacement = decode_2c(BITS_0_7(inst), sizeof(byte));

	/*
	the displacement is relative to the PC+2 (i.e. the word after
	the start of this instruction so let's save the PC now because
	this is where our PC is actually at right now
	*/

	displacement_base = m68k->pc;

	/*
	conditions:
	BRA: 0
	BSR: 1 
	Bcc: ----
	*/

	if(displacement == 0x00) {
		/* a 16-bit displacement follows the instruction */
		PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
		displacement = decode_2c(displacement, sizeof(gword));
	} else if(displacement == 0xFF) {
		dbg_f("Apparently not supported on 68000 (says PRM)");
		/* a 32-bit displacement follows the instruction */
		memory_request_easy(m68k->pc, glong, &displacement);
		/* bump pc two words */
		PM68000_PC_INCX(m68k, 2);
		displacement = decode_2c(displacement, sizeof(glong));
	}

	condition_true = 0;

	switch(condition) {
		case CONDITION_BSR:
			/*
			After a subroutine, execution should continue
			at the next instruction. Our pc will be at that location
			*/
			stack_push(&m68k->system_stack, &m68k->pc);
			/* fall through */
		case CONDITION_BRA:
			condition_true = 1;
			break;
		default:
			condition_true = m68000_test_condition(m68k, condition, cycles);
			break;
	}

	if(condition_true) {
		PM68000_PC_INCX(m68k, displacement - ((m68k->pc) - displacement_base));
	}

	return 0;
}

int m68000_test_condition(struct m68000 *m68k, int condition, int *cycles)
{
	int result;

	switch(condition) {
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

int m68000_exec_andi(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int result, size;
	struct operand_info dest;
	glong source_val, dest_val;

	gword instx[2];

	size = 0;
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE,
					&dest, &size);
	assert(result == 0);

	dest_val = m68000_inst_get_operand_source_val(m68k, mem, &dest, size);
	
	PM68000_GET_NEXT_INSTRUCTION(m68k, &(instx[0]));

	switch(size) {
		case sizeof(byte):
			source_val = WORD_BYTE(instx[0]);
			break;
		case sizeof(gword):
			source_val = instx[0];
			break;
		case sizeof(glong):
			PM68000_GET_NEXT_INSTRUCTION(m68k, &(instx[1]));
			source_val = (instx[0] << (sizeof(gword) * 8)) | (instx[1]);
			break;
	}


	dest_val &= source_val;

	m68000_inst_set_operand_dest(m68k, mem, &dest, dest_val, size);

	if(IS_NEG(dest_val, size)) {
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

	return result;
		
}

int m68000_exec_lea(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int result;
	struct operand_info ea;
	int regno;
	glong ea_val;


	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE,
					&ea, NULL);
	assert(result == 0);

	/* get the Address register to store the result */
	regno = BITS_9_11(inst);

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

	return 0;
}

int m68000_exec_and(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	return m68000_exec_and_or(m68k, mem, inst, cycles, BITWISE_AND);
}
int m68000_exec_or(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	return m68000_exec_and_or(m68k, mem, inst, cycles, BITWISE_OR);
}

/*
The AND and OR operations are the same apart from the bitwise operation being performed
so we will have a common function for both
*/
int m68000_exec_and_or(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles,
				enum BITWISE_OPERATION bop)
{
	int result;
	struct operand_info ea;
	int regno, opmode, size;
	int operation_mode;
	glong operands[2];
	glong and_result;

/* AND operation mode */
#define AND_EA_TO_REG 0
#define AND_REG_TO_EA 1
	
	regno = BITS_9_11(inst);
	opmode = BITS_6_8(inst);

	operation_mode = BITS_3(opmode) >> 2;

	switch(opmode & 3) {
		case 0:
			size = sizeof(byte);
			break;
		case 1:
			size = sizeof(gword);
			break;
		case 2:
			size = sizeof(glong);
			break;
		default:
			dbg_f("Invalid size for operation (%d)", opmode & 3);
			break;
	}

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_ONE,
					&ea, &size);
	assert(result == 0);

	/* get value from given register */
	operands[0] = PM68000_REG_OFF_VAL(m68k, d, regno);

	/* get value from given EA */
	operands[1] = m68000_inst_get_operand_source_val(m68k, mem, &ea, size);

	switch(bop) {
		case BITWISE_AND:
			and_result = operands[0] & operands[1];
			break;
		case BITWISE_OR:
			and_result = operands[0] | operands[1];
			break;
	}

	glong_crop(and_result, size);

	if(operation_mode == AND_EA_TO_REG) {
		m68000_register_set(m68k, M68000_REGISTER_D0 + regno,
					and_result, size);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, and_result, size);
	}
	
	if(IS_NEG(and_result, size)) {
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

int m68000_exec_lsd(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int ir, dr, cr, size, reg, shift_count;
	int shift_type;
	struct operand_info ea;
	int result;
	glong to_shift;

#define LSD_REGISTER_SHIFT 0
#define LSD_MEMORY_SHIFT 1

	result = 0;
	
	dr = BITS_8(inst);

	if(BITS_6_7(inst) == 0x3) {
		shift_type = LSD_MEMORY_SHIFT;
	} else {
		shift_type = LSD_REGISTER_SHIFT;
	}

	if(shift_type == LSD_REGISTER_SHIFT) {

		ir = BITS_5(inst);
		size = m68000_inst_get_operand_size(m68k, mem, inst);
		reg = M68000_REGISTER_D0 + INST_SOURCE_EA_REGISTER(inst);

		to_shift = m68000_register_get(m68k, reg, size, 0);

		/*
		shift count: immediate or register?
		ir = 0: this field contains the shift count.
		ir = 1: shift count = register contents % 64.
		*/
		cr = BITS_9_11(inst);

		if(ir == 0) {
			shift_count = cr;
			if(cr == 0) {
				shift_count = 8;
			}
		} else {
			/*
			presuming the register value does not need 2c decoding as a negative
			value would not make sense
			*/
			shift_count = m68000_register_get(m68k, reg, size, 0);
		}
	} else {
		shift_count = 1;
		size = sizeof(gword);
		result = m68000_inst_get_operand_info(m68k, mem, inst,
						OPERAND_ONE, &ea, &size);
		to_shift = m68000_inst_get_operand_source_val(m68k, mem, &ea, size);

	}

	if(dr == 0) {
		/* right shift */
		if(shift_count > 0) {
			CCR_X_SETX(m68k->status, get_bit(to_shift, shift_count - 1));
			CCR_C_SETX(m68k->status, CCR_X(m68k->status));
		} else {
			CCR_C_UNSET(m68k->status);
		}
		to_shift >>= shift_count;
	} else {
		/* left shift */
		if(shift_count > 0) {
			CCR_X_SETX(m68k->status, 
				get_bit(to_shift, ((size * 8) - 1) - shift_count + 1));
			CCR_C_SETX(m68k->status, CCR_X(m68k->status));
		} else {
			CCR_C_UNSET(m68k->status);
		}
		to_shift <<= shift_count;
	}

	CCR_N_SETX(m68k->status, (to_shift < 0));
	CCR_Z_SETX(m68k->status, (to_shift == 0));
	CCR_Z_UNSET(m68k->status);

	/* now to put the shifted value in the dest */

	if(shift_type == LSD_REGISTER_SHIFT) {
		m68000_register_set(m68k, reg, to_shift, size);
	} else {
		m68000_inst_set_operand_dest(m68k, mem, &ea, to_shift, size);
	}

	return 0;
}
