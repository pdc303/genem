#ifndef __M68000_H__
#define __M68000_H__

#include "types.h"
#include "memory.h"
#include "stack.h"
#include "bitman.h"

#define SYSTEM_STACK_SIZE 65536 // 64k

#define OPERAND_SOURCE 0
#define OPERAND_DEST 1
#define OPERAND_ONE OPERAND_SOURCE

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
/* real opcodes */
#define OPCODE_BITMANIP 0x00
#define OPCODE_MOVEB 0x01
#define OPCODE_MOVEL 0x02
#define OPCODE_MOVEW 0x03
#define OPCODE_MISC 0x04
#define OPCODE_BRANCH 0x06
#define OPCODE_ANDMUL 0x0C
#define OPCODE_ORDIVSBCD 0x08
#define OPCODE_SHIFTROTBIT 0xE

/* fake opcodes */
/*
When a single opcode and represent more than one instruction,
the individual instruction types will be given a fake opcode.

A real opcode is 4 bits long so every fake opcode will have its lower
4 bits equal to the real opcode it falls under, with the higher 4 bits
representing the fake operand in particular
*/

#define GENERATE_FAKE_OPCODE(real, offset) (((offset + 1) << 4) | real)
#define REAL_OPCODE(full) (full & 0xF)

#define OPCODE_TST GENERATE_FAKE_OPCODE(OPCODE_MISC, 0)
#define OPCODE_LEA GENERATE_FAKE_OPCODE(OPCODE_MISC, 1)

#define OPCODE_ANDI GENERATE_FAKE_OPCODE(OPCODE_BITMANIP, 0)

#define OPCODE_MULU GENERATE_FAKE_OPCODE(OPCODE_ANDMUL, 0)
#define OPCODE_ABCD GENERATE_FAKE_OPCODE(OPCODE_ANDMUL, 1)
#define OPCODE_MULS GENERATE_FAKE_OPCODE(OPCODE_ANDMUL, 2)
#define OPCODE_AND GENERATE_FAKE_OPCODE(OPCODE_ANDMUL, 3)
#define OPCODE_EXG GENERATE_FAKE_OPCODE(OPCODE_ANDMUL, 4)

#define OPCODE_OR GENERATE_FAKE_OPCODE(OPCODE_ORDIVSBCD, 0)

/* LSd (LSL, LSR) */
#define OPCODE_LSD GENERATE_FAKE_OPCODE(OPCODE_SHIFTROTBIT, 0)

/* SIZE field values */

#define INST_SIZE(inst) (BITS_6_7(inst))
#define SIZE_FIELD_BYTE 0x01
#define SIZE_FIELD_WORD 0x03
#define SIZE_FIELD_LONG 0x02

/* MISC */

/* is this MISC instruction a TST ? */
//#define INST_MISC_IS_TST(inst) (BITS_6_7(inst) != 3)
#define INST_MISC_IS_TST(inst) ((BITS_6_7(inst) != 0x3) && (BITS_8_15(inst) == 0x4A))
#define INST_MISC_IS_LEA(inst) ((BITS_12_15(inst) == 0x4) && (BITS_6_8(inst) == 0x7))

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

#define INST_BITMANIP_IS_ANDI(inst) (BITS_8_15(inst) == 0x2)

/* ANDMUL */

#define INST_ANDMUL_IS_MULU(inst) (BITS_6_8(inst) == 0x3)
#define INST_ANDMUL_IS_ABCD(inst) (BITS_4_8(inst) == 0x10)
#define INST_ANDMUL_IS_MULS(inst) (BITS_6_8(inst) == 0x7)
#define INST_ANDMUL_IS_AND(inst) (BITS_6_8(inst) != 0x3)
#define INST_ANDMUL_IS_EXG(inst) (BITS_8(inst))

/* ORDIVSBCD */

#define INST_ORDIVSBCD_IS_OR(inst) (BITS_6_8(inst) != 0x3)


/* SHIFTROTBIT */
#define INST_SHIFTROTBIT_IS_LSD(inst) ((BITS_6_7(inst) != 0x3) && (BITS_3_4(inst) == 0x1))

/* affect the PC by one instruction */
#define PM68000_PC_INC(m) ((m->pc) += sizeof(gword))
#define M68000_PC_INC(m) ((m.pc) += sizeof(gword))

/* affect the PC by 'x' bytes */
#define PM68000_PC_INCX(m, x) (m->pc += x)
#define M68000_PC_INCX(m, x) (m.pc += x)

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
#define PM68000_REG_OFF_SET(m, regtype, offset, val) (*(&(m->regtype##0) + offset) = val)

#define PM68000_GET_NEXT_INSTRUCTION(m68k, ret_ptr) \
			PM68000_GET_INSTRUCTION(m68k, 0, ret_ptr)

#define PM68000_GET_INSTRUCTION(m68k, offset, ret_ptr) \
			memory_request_easy(m68k->pc + offset, gword, ret_ptr),\
			PM68000_PC_INC(m68k)

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
	struct stack system_stack;
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
	glong data_int;
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
					struct operand_info *oi, glong val, int size);
int m68000_exec(struct m68000 *m68k, struct memory *mem, int *cycles);
int m68000_decode_opcode(gword inst);
int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size,
							int *cycles);
int m68000_exec_misc(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_tst(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_branch(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_test_condition(struct m68000 *m68k, int condition, int *cycles);
int m68000_exec_bitmanip(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_andi(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_lea(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_and(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_or(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);
int m68000_exec_and_or(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles,
				enum BITWISE_OPERATION bop);
void m68000_register_set(struct m68000 *m68k, enum M68000_REGISTER reg, glong val, int size);
glong m68000_register_get(struct m68000 *m68k, enum M68000_REGISTER reg, int size, int decode);
int m68000_exec_lsd(struct m68000 *m68k, struct memory *mem, gword inst, int *cycles);

#endif /* __M68000_H__ */
