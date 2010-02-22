#include <stdio.h>
#include <config.h>
#include <string.h>
#include <errno.h>

#include "world.h"
#include "memory.h"
#include "cartridge.h"
#include "m68000.h"
#include "debug.h"
#include "sysutil.h"

int main(int argc, char **argv)
{
	int result;
	struct world w;

	world_init(&w, 1024);

	read_file(argv[1], MEMORY_DATA(WORLD_MEM(w)), 1024);

	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w));

	return result;
}
