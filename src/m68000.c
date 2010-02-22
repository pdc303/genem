#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "m68000.h"
#include "types.h"
#include "memory.h"
#include "sysmem.h"
#include "debug.h"
#include "sysutil.h"

int m68000_reset(struct m68000 *m68k)
{
	memset(m68k, 0, sizeof(*m68k));

	return 0;
}

int m68000_exec(struct m68000 *m68k, struct memory *mem)
{
	gword inst;
	int result;
	/* extracted info */
	int ea_mode;
	int ea_register;
	int opcode;

	memory_request_easy(m68k->pc, gword, &inst);

	opcode = INST_OPCODE(inst);

	dbg_i("opcode: %d", opcode);

	switch(opcode) {
		case OPCODE_MOVEB:
			result = m68000_exec_move(m68k, mem,  inst, sizeof(byte));
			break;
		case OPCODE_MOVEL:
			result = m68000_exec_move(m68k, mem, inst, sizeof(glong));
			break;
		case OPCODE_MOVEW:
			result = m68000_exec_move(m68k, mem, inst, sizeof(gword));
			break;
		default:
			dbg_e("Invalid opcode");
			result = 0;
			break;
	}

	return result;

#if 0
	ea_mode = INST_EA_MODE(inst);
	ea_register = INST_EA_REGISTER(inst);


	switch(ea_mode) {
		case 0:
			dbg_i("Mode: Data Register Direct Mode");
			break;
		case 1:
			dbg_i("Mode: Address Register Direct Mode");
			break;
		case 2:
			dbg_i("Mode: Address Register Indirect Mode");
			break;
		case 3:
			dbg_i("Mode: Address Register Indirect with Postincrement Mode");
			break;
		case 4:
			dbg_i("Mode: Address Register Indirect with Predecrement Mode");
			break;
		case 5:
			dbg_i("Mode: Address Register Indirect with Displacement Mode");
			break;
		case 6:
			dbg_i("Mode: Address Register Indirect with Index Mode***");
			/* '6' can be many types of ea_mode. not sure how */
			break;
		case 7:
			if(ea_register == 2)
				dbg_i("Mode: Program Counter Indirect with Displacement Mode***");
			else if(ea_register == 0)
				dbg_i("Mode: Absolute Data Addressing");
			else if(ea_register == 4) {
				gword opcode;
				dbg_i("Mode: Immediate");
				opcode = (inst >> 12) & 0xF;
				//i = i >> 12;
				//i = (i & 0xF);
				dbg_i("opcode: %d", opcode);

			} else if(ea_register == 1) {
				glong addr = 0;
				gword ex1, ex2;
				ex1 = pc[1];
				ex2 = pc[2];

				addr = (((glong) ex1) << 0xF) | ex2;
				addr = be_to_host_glong(addr);
				
				dbg_i("Mode: Absolute Long Addressing Mode (addr: %d)", addr);
			}
			else dbg_i("Mode: ??? (ea_mode 7, ea_register %d)", ea_register);

				
			/* can be others */
			break;
		default:
			dbg_f("Unhandled instruction mode");
	}

	dbg_i("Register: %d", ea_register);

	return result;
#endif
}

int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size)
{
	int opcode;
	int source_mode, source_register, dest_mode, dest_register;
	/* glong is the biggest we will have to cope with for a MOVE */
	glong source_val, dest;
	int result;
	int i;
	
	/* cp will move forward at least one word */ 
	result = 1;

	/* establish operands */

	/* loop 0 will establish the source operand, loop 1 the destination */

	for(i = 0; i < 2; i++) {
		int *ptr;
		int ea_mode, ea_register;

		if(i === 0) {
			ptr = &src_val;
			ea_mode = MOVE_SOURCE_MODE(inst);
			ea_register = MOVE_SOURCE_REGISTER(inst);
		} else {
			ptr = &dest_val;
			ea_mode = MOVE_SOURCE_MODE(inst);
			ea_register = MOVE_SOURCE_REGISTER(inst);
		}

		ptr = i ? &dest_val : &src_val;



	switch(source_mode) {
		case EA_MODE_DATA_REGISTER_DIRECT:
			/* get the value at the specified data register */
			//source_val = *((&(m68k->d0)) + source_register);
			source_val = PM68000_REG_OFF_VAL(m68k, d, source_register);
			break;
		case EA_MODE_ADDRESS_REGISTER_DIRECT:
			dbg_e("ADDRESS_REGISTER_DIRECT not allowed for MOVE source");
			goto m68000_exec_move_out_error;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_PREDEC:
			PM68000_REG_OFF_DECX(m68k, a, source_register, size);
			/* now fall through to EA_MODE_ADDRESS_REGISTER_INDIRECT */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC:
			/* if POSTINC, do the normal INDIRECT action here then increment */
			/* ... fall through ... */
		case EA_MODE_ADDRESS_REGISTER_INDIRECT:
			/* get the value at the location given at this address register */
			memory_request(mem, PM68000_REG_OFF_VAL(m68k, a, source_register),
							size, &source_val, 1);
			if(source_mode == EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC) {
				PM68000_REG_OFF_INCX(m68k, a, source_register, size);
			}
			break;
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_DISPLACEMENT:
		{
			gword displacement;
			PM68000_GET_INSTRUCTION(m68k, 1, &displacement);
			/* sign extension omitted. don't think I need it. */
			memory_request(mem,
				PM68000_REG_OFF_VAL(m68k, a, source_register) + displacement,
							size, &source_val, 1);
			break;
		}
		case EA_MODE_ADDRESS_REGISTER_INDIRECT_INDEX:
			dbg_f("MOVE source with Index not yet implemented");
			break;
		case EA_MODE_NOREG:
			switch(source_register) {
				case EA_REGISTER_PROGRAM_COUNTER_INDIRECT_DISPLACEMENT:
				{
					gword displacement;
					PM68000_GET_INSTRUCTION(m68k, 1, &displacement);
					memory_request(mem, ((size_t) m68k->pc) + displacement, size, &source_val, 1);
					break;
				}
				case EA_REGISTER_PROGRAM_COUNTER_INDIRECT_INDEX:
					dbg_f("MOVE source with Index not yet implemented");
					goto m68000_exec_move_out_error;
				case EA_REGISTER_ABSOLUTE_SHORT_ADDRESS:
				{
					gword address;
					PM68000_GET_INSTRUCTION(m68k, 1, &address);
					memory_request(mem, address, size, &source_val, 1);
					goto m68000_exec_move_found_source;
				}
				case EA_REGISTER_ABSOLUTE_LONG_ADDRESS:
				{
					gword address_high;
					gword address_low;
					gword address;
					PM68000_GET_INSTRUCTION(m68k, 1, &address_high);
					PM68000_GET_INSTRUCTION(m68k, 2, &address_low);
					address = (address_high << sizeof(gword)) | (address_low);
					memory_request(mem, address, size, &source_val, 1);
					goto m68000_exec_move_found_source;
				}
				case EA_REGISTER_IMMEDIATE_DATA:
				{
					/* there will be a max of 6 extension words */
					gword ext[2];
					PM68000_GET_INSTRUCTION(m68k, 1, &(ext[0]));
					switch(size) {
						case sizeof(byte):
							source_val = WORD_0_7(ext[0]);
							break;
						case sizeof(gword):
							source_val = ext[0];
							break;
						case sizeof(glong):
							PM68000_GET_INSTRUCTION(m68k, 2, &(ext[1]));
							source_val = (ext[0] << sizeof(gword))
									& ext[1];
							break;
						default:
							dbg_e("Invalid size for MOVE src Immediate data");
							goto m68000_exec_move_out_error;
							

					}
					goto m68000_exec_move_found_source;
				}
			}
			
		default:
			dbg_e("MOVE source mode not implemented");
			goto m68000_exec_move_out_error;
	}

m68000_exec_move_found_source:

	dest_mode = MOVE_DEST_MODE(inst);
	dest_register = MOVE_DEST_REGISTER(inst);



m68000_exec_move_out_error:
	return -result;
}

int m68000_run_file(const char *filename)
{
	size_t filesize;
	byte *data;
	FILE *fp;
	int result;
	struct m68000 m68k;

	data = NULL;
	fp = NULL;

	filesize = get_file_size(filename);

	if(filesize < 0) {
		dbg_e("Unable to open file: %s", filename);
		result = -EIO;
		goto m68000_run_file_out;
	}

	fp = fopen(filename, "r");

	data = genem_malloc(filesize);
	fread(data, 1, filesize, fp);
	
//	m68000_exec(&m68k, mem, (gword *) data);

m68000_run_file_out:
	if(data != NULL) {
		genem_free(data);
	}

	if(fp != NULL) {
		fclose(fp);
	}

	return result;
}
