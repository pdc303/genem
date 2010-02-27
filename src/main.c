#include <stdio.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>
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
	struct cartridge *cart;
	int cycles;
	
	world_init(&w);

	if(get_file_size(argv[1]) > 512) {
		cartridge_load_from_file(argv[1], &cart);
		WORLD_M68K(w).pc = be_to_host_glong(cart->pc);
	} else {
	}
	read_file(argv[1], MEMORY_DATA(WORLD_MEM(w)), 1000000);

	cycles = 0;
	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);
	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);
	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);
	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);
	m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);

	return result;
}
