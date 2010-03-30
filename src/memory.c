#include <string.h>
#include <errno.h>
#include <assert.h>

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
int memory_request(struct memory *mem, size_t address, void *n, int size,
			int convert, gclock_t *cycles)
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

gword memory_request_gword(struct memory *mem, size_t address, int convert,
				gclock_t *cycles)
{
	gword out;
	int r;

	r = memory_request(mem, address, &out, sizeof(gword), convert, cycles);
	assert(r == 0);

	return out;
}

glong memory_request_glong(struct memory *mem, size_t address, int convert,
				gclock_t *cycles)
{
	glong out;
	int r;

	r = memory_request(mem, address, &out, sizeof(glong), convert, cycles);
	assert(r == 0);

	return out;
}

int memory_request_multi(struct memory *mem, size_t address, void *n, int size,
					int num, int convert, gclock_t *cycles)
{
	int i, result;

	result = 0;

	for(i = 0; i < num; i++) {
		result |= memory_request(mem, address, ((byte *) n) + (i * size),
						size, convert, cycles);
	}

	return result;
}

int memory_write(struct memory *mem, size_t address, void *n, int size,
			int convert, gclock_t *cycles)
{
	if((( address) + size) > mem->len) {
		dbg_e("Memory request out of bounds (%ld + %d > %ld)",
				address, size, mem->len);
		return -EFAULT;
	}
	
	if(convert) {
		swap_bytes(n, size);
	}
	
	memcpy(PMEMORY_DATA(mem) + address, n, size);

	return 0;
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

glong host_to_be_glong(glong n)
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
