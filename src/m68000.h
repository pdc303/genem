#ifndef __M68000_H__
#define __M68000_H__

#include "types.h"
#include "memory.h"

#define OPERAND_SOURCE 0
#define OPERAND_DEST 1

/* expose the Effective Address Register (sub)field of an instruction word */
#define INST_SOURCE_EA_REGISTER(n) (n & 0x07)
#define INST_SOURCE_EA_MODE(n) ((n >> 3) & 0x07)
#define INST_DEST_EA_MODE(n) ((n >> 6) & 0x07)
#define INST_DEST_EA_REGISTER(n) ((n >> 9) & 0x07)

/* extract the opcode bits from a (HE) word */
#define INST_OPCODE(n) ((n >> 12) & 0xF)

/* resolve true if the opcode is that of a 'MOVE' */
#define INST_OPCODE_IS_MOVE(n) ((!(n & 0x3)) && (n))

/* opcodes bit patterns corresponding numbers */
#define OPCODE_MOVEB 0x01
#define OPCODE_MOVEL 0x02
#define OPCODE_MOVEW 0x03
#define OPCODE_MISC 0x04
#define OPCODE_BRANCH 0x6

/* SIZE field values */

#define INST_SIZE(inst) (BITS_6_7(inst))
#define SIZE_FIELD_BYTE 0x01
#define SIZE_FIELD_WORD 0x03
#define SIZE_FIELD_LONG 0x02

/* MISC */

/* is this MISC instruction a TST ? */
#define INST_MISC_IS_TST(inst) (BITS_6_7(inst) != 3)

/* BRANCH */
#define CONDITION_BRA 0x0
#define CONDITION_BSR 0x1
#define CONDITION_HI 0x2
#define CONDITION_LS 0x3
#define CONDITION_CC_HI 0x4
#define CONDITION_CC_LO 0x5
#define CONDITION_NE 0x6
#define CONDITION_EQ 0x7
#define CONDITION_VC 0x8
#define CONDITION_VS 0x9
#define CONDITION_PL 0xA
#define CONDITION_MI 0xB
#define CONDITION_GE 0xC
#define CONDITION_LT 0xD
#define CONDITION_GT 0xE
#define CONDITION_LE 0xF


#define PM68000_PC_INC(m) ((m->pc) += sizeof(gword))
#define M68000_PC_INC(m) ((m.pc) += sizeof(gword))
#define PM68000_PC_INCX(m, x) ((m->pc) += (x * sizeof(gword)))
#define M68000_PC_INCX(m, x) ((m.pc) += (x * sizeof(gword)))

/* increase or decrease an m68k register value by x */
#define PM68000_REG_INCX(m, reg, x) (m->reg += x)
#define PM68000_REG_DECX(m, reg, x) PM68000_REG_INCX(m, reg, -x);

/*
increase or decrease an m68k register value by x.
This macro makes it possible to adjust the value of a register by
simply knowing its offset. e.g. to increase register d4 by 6:
PM68000_REG_OFF_INCX(m, d, 4, 6)
*/

#define PM68000_REG_OFF_INCX(m, regtype, offset, x) \
			(*((&(m->regtype##0)) + offset) += x)
#define PM68000_REG_OFF_DECX(m, regtype, offset, x) \
			PM68000_REG_OFF_INCX(m, regtype, offset, -x)

/* get a pointer to a register when you know its type and offset */
#define PM68000_REG_OFF_PTR(m, regtype, offset) (&(m->regtype##0) + offset)
/* get the value of a register when you know its type and offset */
#define PM68000_REG_OFF_VAL(m, regtype, offset) (*(&(m->regtype##0) + offset))

#define PM68000_GET_NEXT_INSTRUCTION(m68k, ret_ptr) \
			PM68000_GET_INSTRUCTION(m68k, 0, ret_ptr)

#define PM68000_GET_INSTRUCTION(m68k, offset, ret_ptr) \
			memory_request_easy(m68k->pc + offset, gword, ret_ptr),\
			PM68000_PC_INC(m68k)

/* macros to extract numbers from numbers */
/* get Byte from a Word */
#define WORD_BYTE(w) BITS_0_7(w)
/* get bits 0 to 7 of a value */
#define BITS_0_7(w) (w & 0xFF)
/* get bits 6 and 7 of a value */
#define BITS_6_7(w) ((w & 0xC0) >> 6)
#define BITS_8_11(w) ((w & 0xF00) >> 8)
#define BITS_12_13(w) ((w & 0x3000) >> 12)
#define BITS_7(w) ((w & 0x80) >> 7)
#define BITS_15(w) ((w & 0x8000) >> 15)
#define BITS_31(w) ((w & 0x80000000) >> 31)

#define LONG_BYTE(l) WORD_BYTE(l)
#define LONG_WORD(l) (l & 0xFFFF)

/* effective address modes */

/* operand is in the given data register */
#define EA_MODE_DATA_REGISTER_DIRECT 0x00
/* operand is in the given address register */
#define EA_MODE_ADDRESS_REGISTER_DIRECT 0x01
/* operand is at the address held by the given address register */
#define EA_MODE_ADDRESS_REGISTER_INDIRECT 0x02
/*
operand is at the address held by the given address register.
after using the address, it is incremented by the size of the operand.
NB: if the address register is a stack pointer and the operand size is 'byte',
the address is incremented by two to keep the stack pointer aligned to a word
boundary.
*/
#define EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC 0x03
/*
As EA_MODE_ADDRESS_REGISTER_INDIRECT_POSTINC but the address is
decremented before it is used
*/
#define EA_MODE_ADDRESS_REGISTER_INDIRECT_PREDEC 0x04
/*
Operand is at the memory address calculated by adding:
	the address in the given address register
	+
	the 16-bit integer in the extension word sign-extended to 32-bits
*/
#define EA_MODE_ADDRESS_REGISTER_INDIRECT_DISPLACEMENT 0x05
/*
Operand location is the sum of:-

	the address in the given address register
	+
	displacement value (extension word's low-order 8 bits)
	+
	the index register's sign-extended contents (to 32-bit)
	...umm
	
	Should we be using the Brief Extension word format on PRM Page 2-2?
*/
#define EA_MODE_ADDRESS_REGISTER_INDIRECT_INDEX 0x06
/*
This Source Mode requires inspection of the Source Register value to find out
how the Source value should be obtained.
*/
#define EA_MODE_NOREG 0x07
/*
The operand location in memory is the sum of the program counter and the
sign-extended 16-bit displacement integer in the extension word
*/
#define EA_REGISTER_PROGRAM_COUNTER_INDIRECT_DISPLACEMENT 0x02
/* todo */
#define EA_REGISTER_PROGRAM_COUNTER_INDIRECT_INDEX 0x03
/* operand is at the given address */
#define EA_REGISTER_ABSOLUTE_SHORT_ADDRESS 0x00
/* operand is at the given address */
#define EA_REGISTER_ABSOLUTE_LONG_ADDRESS 0x01
/* operand is given */
#define EA_REGISTER_IMMEDIATE_DATA 0x04

/* for when we have an iteratio to deal with source and one for dest */

#define LOOP_SOURCE 0
#define LOOP_DEST 1

/* get CCR bits */
/*
these macros won't necessarily produce '1' if the given flag is set
but will produce non-zero and as such are appropriate for testing
*/

#define CCR_C_BITVAL 1
#define CCR_V_BITVAL 2
#define CCR_Z_BITVAL 4
#define CCR_N_BITVAL 8
#define CCR_X_BITVAL 16

#define CCR_C(status) (status & CCR_C_BITVAL)
#define CCR_V(status) (status & CCR_V_BITVAL)
#define CCR_Z(status) (status & CCR_Z_BITVAL)
#define CCR_N(status) (status & CCR_N_BITVAL)
#define CCR_X(status) (status & CCR_X_BITVAL)

/* set/unset a CCR flag */

#define CCR_C_SET(status) (status |= CCR_C_BITVAL)
#define CCR_V_SET(status) (status |= CCR_V_BITVAL)
#define CCR_Z_SET(status) (status |= CCR_Z_BITVAL)
#define CCR_N_SET(status) (status |= CCR_N_BITVAL)
#define CCR_X_SET(status) (status |= CCR_X_BITVAL)

#define CCR_C_UNSET(status) (status ^= CCR_C_BITVAL)
#define CCR_V_UNSET(status) (status ^= CCR_V_BITVAL)
#define CCR_Z_UNSET(status) (status ^= CCR_Z_BITVAL)
#define CCR_N_UNSET(status) (status ^= CCR_N_BITVAL)
#define CCR_X_UNSET(status) (status ^= CCR_X_BITVAL)



enum M68000_REGISTER {
	M68000_REGISTER_D0,
	M68000_REGISTER_D1,
	M68000_REGISTER_D2,
	M68000_REGISTER_D3,
	M68000_REGISTER_D4,
	M68000_REGISTER_D5,
	M68000_REGISTER_D6,
	M68000_REGISTER_D7,
	
	M68000_REGISTER_A0,
	M68000_REGISTER_A1,
	M68000_REGISTER_A2,
	M68000_REGISTER_A3,
	M68000_REGISTER_A4,
	M68000_REGISTER_A5,
	M68000_REGISTER_A6,
	M68000_REGISTER_A7,
	
	M68000_REGISTER_FP0,
	M68000_REGISTER_FP1,
	M68000_REGISTER_FP2,
	M68000_REGISTER_FP3,
	M68000_REGISTER_FP4,
	M68000_REGISTER_FP5,
	M68000_REGISTER_FP6,
	M68000_REGISTER_FP7,
	
	M68000_REGISTER_NUM //number of registers
};


struct m68000
{
	glong d0;
	glong d1;
	glong d2;
	glong d3;
	glong d4;
	glong d5;
	glong d6;
	glong d7;
	
	glong a0;
	glong a1;
	glong a2;
	glong a3;
	glong a4;
	glong a5;
	glong a6;
	glong a7; // USP
	
	long double fp0;
	long double fp1;
	long double fp2;
	long double fp3;
	long double fp4;
	long double fp5;
	long double fp6;
	long double fp7;

	glong pc;

	/*
	Status Register. the lower byte of the status register is the
	Condition Code Register.
	*/
	gword status;

	/*
	For the time being we will have the biggest floating point type
	available. This may require us to keep an eye on what should be
	an overflow and other limitations we need to impose.
	*/


	gword fpcr;
	glong fpsr;
	glong fpiar;
	void *register_pointers[M68000_REGISTER_NUM];
};

enum OPERAND_TYPE {
	OPERAND_TYPE_REGISTER,
	OPERAND_TYPE_INDIRECT,
	OPERAND_TYPE_IMMEDIATE,
	OPERAND_TYPE_INVALID
};

struct operand_info
{
	int type;
	int reg;
	//glong *reg_ptr;
	glong address;
	int data_int;
};

/* reset an m68k to an all-zero state */
int m68000_reset(struct m68000 *m68k);
/* initialise the register pointers array for this m68k */
int m68000_init_register_pointers_array(struct m68000 *m68k);
int m68000_inst_get_operand_info(struct m68000 *m68k, struct memory *mem, gword inst,
				int operand_type, struct operand_info *oi, int *size);
int m68000_inst_get_operand_size(struct m68000 *m68k, struct memory *mem, gword inst);
int m68000_inst_get_operand_source_val(struct m68000 *m68k, struct memory *mem,
							struct operand_info *oi, int size);
int m68000_inst_set_operand_dest(struct m68000 *m68k, struct memory *mem,
					struct operand_info *oi, int size, int val);
int m68000_exec(struct m68000 *m68k, struct memory *mem, int *cycles);
int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size,
							int *cycles);
int m68000_exec_misc(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_tst(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_branch(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_test_condition(struct m68000 *m68k, int condition, int *cycles);

#endif /* __M68000_H__ */
