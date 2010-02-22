#ifndef __M68000_H__
#define __M68000_H__

#include "types.h"
#include "memory.h"

/* expose the Effective Address field of an instruction word */
#define INST_EA_MASK 0x3F

/* expose the Effective Address Mode (sub)field of an instruction word */
#define INST_EA_MODE_MASK 0x38
#define INST_EA_MODE(n) ((n & INST_EA_MODE_MASK) >> 3)

/* expose the Effective Address Register (sub)field of an instruction word */
#define INST_EA_REGISTER_MASK 0x07
#define INST_EA_REGISTER(n) (n & INST_EA_REGISTER_MASK)

/* extract the opcode bits from a (HE) word */
#define INST_OPCODE(n) ((n >> 12) & 0xF)

/* resolve true if the opcode is that of a 'MOVE' */
#define INST_OPCODE_IS_MOVE(n) ((!(n & 0x3)) && (n))

/* opcodes bit patterns corresponding numbers */
#define OPCODE_MOVEB 0x1
#define OPCODE_MOVEL 0x2
#define OPCODE_MOVEW 0x3

#define MOVE_SOURCE_REGISTER(n) (n & 0x7)
#define MOVE_SOURCE_MODE(n) ((n >> 3) & 0x7)
#define MOVE_DEST_MODE(n) ((n >> 6) & 0x7)
#define MOVE_DEST_REGISTER(n) ((n >> 9) & 0x7)

#define PM68000_PC_INC(m) ((m->pc)++)
#define M68000_PC_INC(m) ((m.pc)++)
#define PM68000_PC_INCX(m, x) ((m->pc) += x)
#define M68000_PC_INCX(m, x) ((m.pc) += x)

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

#define PM68000_GET_INSTRUCTION(m68k, offset, ret_ptr) \
			memory_request_easy(m68k->pc + offset, gword, ret_ptr)

/* macros to extract numbers from numbers */
/* get bits 0 to 7 of a word */
#define WORD_0_7(w) (w & 0xFF)

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

	gword *pc;
	byte ccr;

	/*
	For the time being we will have the biggest floating point type
	available. This may require us to keep an eye on what should be
	an overflow and other limitations we need to impose.
	*/

	long double fp0;
	long double fp1;
	long double fp2;
	long double fp3;
	long double fp4;
	long double fp5;
	long double fp6;
	long double fp7;

	gword fpcr;
	glong fpsr;
	glong fpiar;
};

/* reset an m68k to an all-zero state */
int m68000_reset(struct m68000 *m68k);
int m68000_exec(struct m68000 *m68k, struct memory *mem);
int m68000_exec_move(struct m68000 *m68k, struct memory *mem, gword inst, int size);
int m68000_run_file(const char *filename);

#endif /* __M68000_H__ */
