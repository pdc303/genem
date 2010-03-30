#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>

#include "types.h"
#include "config.h"

#define GENESIS_MEMORY_LEN 0xFFFFFF

/*
endianness definition
uses autoconf's WORDS_BIGENDIAN and makes a #define I'm happier with
*/

#ifndef WORDS_BIGENDIAN
	/* Little-Endian Host */
	#define HOST_LE
#endif

#define MEMORY_DATA(mem) (mem.data)
#define PMEMORY_DATA(mem) (mem->data)


struct memory
{
	/* size of memory in bytes */
	size_t len;
	byte *data;
};

int memory_init(struct memory *mem);
/*
sets the memory pointed to by 'n' to the data at the given memory address.
'size' bytes will be written so 'n' must have enough bytes allocated.
if 'convert' is non-zero, the value will be converted to host endinanness.
*/
int memory_request(struct memory *mem, size_t address, void *n, int size,
			int convert, gclock_t *cycles);
/* XXX: comment */
int memory_request_multi(struct memory *mem, size_t address, void *n, int size,
					int num, int convert, gclock_t *cycles);
gword memory_request_gword(struct memory *mem, size_t address, int convert,
				gclock_t *cycles);
glong memory_request_glong(struct memory *mem, size_t address, int convert,
				gclock_t *cycles);
int memory_write(struct memory *mem, size_t address, void *n, int size,
			int convert, gclock_t *cycles);
/* returns (by pointer) the value at the requested address */
#define memory_request_easy(addr, type, ret_ptr) \
			memory_request(mem, (size_t) (addr), ret_ptr, sizeof(type), 1, &m68k->cycles)
#define memory_request_multi_easy(addr, type, num, ret_ptr) \
		memory_request_multi(mem, (size_t) (addr), ret_ptr, sizeof(type), num, 1, cycles)

void swap_bytes(byte *b, int len);
gint be_to_host_gint(gint n);
glong be_to_host_glong(glong n);
gword be_to_host_gword(gword n);
glong host_to_be_glong(glong n);
#endif /* __MEMORY_H__ */
