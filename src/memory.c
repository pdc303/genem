#include <string.h>
#include <errno.h>

#include "types.h"
#include "memory.h"
#include "sysmem.h"
#include "debug.h"
#include "bitman.h" /* bit extraction macros */

int memory_init(struct memory *mem)
{
	mem->len = GENESIS_MEMORY_LEN;
	mem->data = genem_zalloc(mem->len);



	return 0;
}

int memory_request(struct memory *mem, size_t address, int size, void *n, int convert)
{
	if((( address) + size) > mem->len) {
		dbg_e("Memory request out of bounds (%ld + %d > %ld)",
				address, size, mem->len);
		return -EFAULT;
	}

	memcpy(n, PMEMORY_DATA(mem) + address, size);

	if(convert) {
		swap_bytes(n, size);
	}

	return 0;
}

int memory_request_multi(struct memory *mem, size_t address, int size,
						int num, void *n, int convert)
{
	int i, result;

	result = 0;

	for(i = 0; i < num; i++) {
		result |= memory_request(mem, address, size,
					((byte *) n) + (i * size), convert);
	}

	return result;
}

gint be_to_host_gint(gint n)
{
#ifdef HOST_LE
	gint result;
	result = n;
	swap_bytes((byte *) &result, sizeof(result));
	return result;
#else 
	return n;
#endif
}

glong be_to_host_glong(glong n)
{
#ifdef HOST_LE
	glong result;
	result = n;
	swap_bytes((byte *) &result, sizeof(result));
	return result;
#else 
	return n;
#endif
}

gword be_to_host_gword(gword n)
{
#ifdef HOST_LE
	gword result;
	result = n;
	swap_bytes((byte *) &result, sizeof(result));
	return result;
#else 
	return n;
#endif
}

void swap_bytes(byte *b, int len)
{
	byte temp;
	int i;

	for(i = 0; i < len; i += 2) {
		temp = b[i];
		b[i] = b[len - 1 - i];
		b[len - 1 - i] = temp;
	}
}

