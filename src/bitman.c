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

int get_bit(int src, int bitno)
{
	return (src >> bitno) & 0x01;
}
