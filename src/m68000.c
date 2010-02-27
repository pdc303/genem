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

int m68000_reset(struct m68000 *m68k)
{
	memset(m68k, 0, sizeof(*m68k));
	m68000_init_register_pointers_array(m68k);

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

	opcode = INST_OPCODE(inst);

	//dbg_i("opcode: %d", opcode);

	switch(opcode) {
		case OPCODE_MOVEB:
			result = m68000_exec_move(m68k, mem,  inst, sizeof(byte), cycles);
			break;
		case OPCODE_MOVEL:
			result = m68000_exec_move(m68k, mem, inst, sizeof(glong), cycles);
			break;
		case OPCODE_MOVEW:
			result = m68000_exec_move(m68k, mem, inst, sizeof(gword), cycles);
			break;
		case OPCODE_MISC:
			result = m68000_exec_misc(m68k, mem, inst, cycles);
			break;
		case OPCODE_BRANCH:
			result = m68000_exec_branch(m68k, mem, inst, cycles);
			break;
		default:
			dbg_e("Unhandled opcode: %d", opcode);
			result = -1;
			break;
	}

	return result;
}

int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size,
							int *cycles)
{
	int result;
	struct operand_info src, dest;
	/* longest source value will be a long */
	glong source_val;

	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_SOURCE, &src, NULL);
	assert(result == 0);
	result = m68000_inst_get_operand_info(m68k, mem, inst, OPERAND_DEST, &dest, NULL);
	assert(result == 0);

	source_val = m68000_inst_get_operand_source_val(m68k, mem, &src, size);
	m68000_inst_set_operand_dest(m68k, mem, &dest, size, source_val);
	
	switch(dest.type) {
		case OPERAND_TYPE_REGISTER:
			*(glong *) (m68k->register_pointers[dest.reg]) = source_val;
			break;
		case OPERAND_TYPE_INDIRECT:
			//memory_put
			break;
		case OPERAND_TYPE_IMMEDIATE:
			dbg_e("Immediate data destination invaid for MOVE");
			break;
		default:
			dbg_f("Unknown dest operand type");
			break;
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

	return 0;

//m68000_exec_move_out_error:
	return result;
}

int m68000_inst_get_operand_info(struct m68000 *m68k, struct memory *mem, gword inst,
					int operand_type, struct operand_info *oi, int *size)
{
	int ea_mode, ea_register;
	int local_size;

	memset(oi, 0, sizeof(*oi));

	local_size = m68000_inst_get_operand_size(m68k, mem, inst);

	if(size != NULL) {
		*size = local_size;
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
					gword displacement;
					PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
					oi->type = OPERAND_TYPE_INDIRECT;
					oi->address = (size_t) (m68k->pc + displacement);
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

	switch(size) {
		case sizeof(byte):
			source_val = LONG_BYTE(source_val);
			break;
	}

	return decode_integer(source_val, size);
}

int m68000_inst_set_operand_dest(struct m68000 *m68k, struct memory *mem,
					struct operand_info *oi, int size, int val)
{	
	switch(oi->type) {
		case OPERAND_TYPE_REGISTER:
			*(glong *) (m68k->register_pointers[oi->reg]) = val;
			break;
		case OPERAND_TYPE_INDIRECT:
			//memory_put
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

	opcode = INST_OPCODE(inst);

	switch(opcode) {
		case OPCODE_MOVEB:
		case OPCODE_MOVEL:
		case OPCODE_MOVEW:
			size_format = 1;
			size = BITS_12_13(inst);
			break;
		case OPCODE_MISC:
			if(INST_MISC_IS_TST(inst)) {
				size_format = 2;
				size = BITS_6_7(inst);
			}
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
	}

//m68000_inst_get_operand_size_out:
	return bytes;
}

int m68000_exec_misc(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int result;

	if(INST_MISC_IS_TST(inst)) {
		result = m68000_exec_tst(m68k, mem, inst, cycles);
	} else {
		dbg_e("Unhandled MISC instruction");
	}

//m68000_exec_misc_out:
	return result;
}

int m68000_exec_tst(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles)
{
	int size;
	int result;
	struct operand_info src;
	int source_val;

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
	glong displacement;
	int condition_true;

	condition = BITS_8_11(inst);
	displacement = decode_integer(BITS_0_7(inst), sizeof(byte));

	/*
	conditions:
	BRA: 0
	BSR: 1 
	Bcc: ----
	*/

	if(displacement == 0x00) {
		/* a 16-bit displacement follows the instruction */
		PM68000_GET_NEXT_INSTRUCTION(m68k, &displacement);
		displacement = decode_integer(displacement, sizeof(gword));
	} else if(displacement == 0xFF) {
		dbg_f("Apparently not supported on 68000 (says PRM)");
		/* a 32-bit displacement follows the instruction */
		memory_request_easy(m68k->pc, glong, &displacement);
		/* bump pc two words */
		PM68000_PC_INCX(m68k, 2);
		displacement = decode_integer(displacement, sizeof(glong));
	}

	condition_true = 0;




	switch(condition) {
		case CONDITION_BRA:
			condition_true = 1;
			break;
		case CONDITION_BSR:
			dbg_f("BSR: TODO");
			break;
		default:
			condition_true = m68000_test_condition(m68k, condition, cycles);
			break;
	}

	return 0;
}

int m68000_test_condition(struct m68000 *m68k, int condition, int *cycles)
{
	int result;

	/*

	^  AND
	V  OR
	(+) XOR
	*/

	result = 0;

	switch(condition) {
		case CONDITION_HI:
			dbg_i("");
			break;
		case CONDITION_LS:
			dbg_i("");
			break;
		case CONDITION_CC_HI:
			dbg_i("");
			break;
		case CONDITION_CC_LO:
			dbg_i("");
			break;
		case CONDITION_NE:
			result = CCR_Z(m68k->status);
			dbg_i("");
			break;
		case CONDITION_EQ:
			result = !CCR_Z(m68k->status);
			dbg_i("");
			break;
		case CONDITION_VC:
			dbg_i("");
			break;
		case CONDITION_VS:
			dbg_i("");
			break;
		case CONDITION_PL:
			dbg_i("");
			break;
		case CONDITION_MI:
			dbg_i("");
			break;
		case CONDITION_GE:
			dbg_i("");
			break;
		case CONDITION_LT:
			dbg_i("");
			break;
		case CONDITION_GT:
			dbg_i("");
			break;
		case CONDITION_LE:
			dbg_i("");
			break;
		default:
			dbg_e("Unsupported condition type");
	}

	return result;
}
