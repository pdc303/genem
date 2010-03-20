#include "bitman.h"
#include "types.h"
#include "debug.h"

glong glong_crop(glong n, int size)
{
	glong result;

	switch(size) {
		case sizeof(byte):
			result = LONG_BYTE(n);
			break;
		case sizeof(gword):
			result = LONG_WORD(n);
			break;
		case sizeof(glong):
			result = n;
			break;
	}

	return result;
}

glong encode_2c(glong n, int size)
{
	return glong_crop(n, size);
}

glong decode_2c(glong val, int size)
{
	int is_neg;
	glong inv;
	glong result;

	/* save any work if it's zero */
	if(val == 0) {
		return 0;
	}

	is_neg = 0;
	inv = (~val) + 1;

	switch(size) {
		case sizeof(byte):
			if(!BITS_7(val)) {
				result = val;
			} else {
				result = -LONG_BYTE(inv);
			}
			break;
		case sizeof(gword):
			if(!BITS_15(val)) {
				result = val;
			} else {
				result = -LONG_WORD(inv);
			}
			break;
		case sizeof(glong):
			if(!BITS_31(val)) {
				result = val;
			} else {
				result = -inv;
			}
			break;
		default:
			dbg_e("Invalid size");
			return 0;
	}

	return result;
}

int get_bit(glong src, int bitno)
{
	return (src >> bitno) & 0x01;
}

glong set_bit(glong x, int bitno)
{
	return x | (1 << bitno);
}

glong unset_bit(glong x, int bitno)
{
	return x & (~(1 << bitno));
}

glong set_bit_to_val(glong x, int bitno, int val)
{
	return val ? set_bit(x, bitno) : unset_bit(x, bitno);
}

/* rotate bits high..low in x. ensure bit 'protect' is not changed */
glong rotate_bits(glong x, int high, int low, int count, enum LR lr)
{
	int save;
	int i;
	int mask;
	int orig, integrater;

	orig = x;

	/* create a 'mask' to represent the bits which are subject to change */
	mask = 0;
	for(i = low; i <= high; i++) {
		mask = set_bit(mask, i);
	}
	
	/* create a 'mask' which is the original value but with the bits subject to change 0 */
	integrater = orig;
	for(i = low; i <= high; i++) {
		integrater = unset_bit(integrater, i);
	}

	while(count--) {
		if(lr == LEFT) {
			save = get_bit(x, high);
			x <<= 1;
			x = set_bit_to_val(x, low, save);
		} else {
			save = get_bit(x, low);
			x >>= 1;
			x = set_bit_to_val(x, high, save);
		}
	}
	/* mask out any bits we altered but were not actually part of the rotation */
	x &= mask;

	/* apply the changes to the original value */
	integrater |= x;

	return integrater;
}
