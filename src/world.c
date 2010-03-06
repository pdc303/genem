#include "world.h"

int world_init(struct world *w)
{
	int result;

	result = 0;

	result = m68000_init(PWORLD_PM68K(w));

	result |= memory_init(PWORLD_PMEM(w));

	return result;
}
