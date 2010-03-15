#ifndef __BITMAN_H__
#define __BITMAN_H__

#include "types.h"

/* macros to extract numbers from numbers */
/* get Byte from a Word */
#define WORD_BYTE(w) BITS_0_7(w)
/* get bits 0 to 7 of a value */
#define BITS_0_7(w) (w & 0xFF)
#define BITS_3(w) ((w >> 3) & 0x01)
#define BITS_3_4(w) ((w >> 3) & 0x3)
#define BITS_3_7(w) ((w >> 3) & 0x1F)
#define BITS_4_8(w) ((w >> 4) & 0x1F)
#define BITS_5(w) ((w >> 5) & 0x1)
/* get bits 6 and 7 of a value */
#define BITS_6_7(w) ((w & 0xC0) >> 6)
#define BITS_6_8(w) ((w >> 6) & 0x7)
#define BITS_7(w) ((w & 0x80) >> 7)
#define BITS_8(w) ((w >> 8) & 0x01)
#define BITS_8_15(w) ((w & 0xFFFFFF00) >> 8)
#define BITS_8_11(w) ((w & 0xF00) >> 8)
#define BITS_9_11(w) ((w >> 9) & 0x07)
#define BITS_9_15(w) ((w >> 9) & 0x7F)
#define BITS_12_13(w) ((w & 0x3000) >> 12)
#define BITS_12_15(w) ((w & 0xFFFFF000) >> 12)
#define BITS_15(w) ((w & 0x8000) >> 15)
#define BITS_31(w) ((w & 0x80000000) >> 31)

#define LONG_BYTE(l) WORD_BYTE(l)
#define LONG_WORD(l) (l & 0xFFFF)

/* return non-zero if a number is negative */
#define IS_NEG(n, size) (n >> ((size * 8) - 1) & 1)

enum BITWISE_OPERATION {
		BITWISE_OR,
		BITWISE_AND
};

glong glong_crop(glong n, int size);
glong encode_2c(glong n, int size);
glong decode_2c(glong val, int size);
/* get bit 'bitno' from 'src' */
int get_bit(int src, int bitno);

#endif
