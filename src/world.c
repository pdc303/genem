#include "world.h"

int world_init(struct world *w, size_t mem_len)
{
	int result;

	result = 0;

	result = m68000_reset(PWORLD_PM68K(w));

	result |= memory_init(PWORLD_PMEM(w), mem_len);

	return result;
}
