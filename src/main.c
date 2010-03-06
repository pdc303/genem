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
	int op, real;

	world_init(&w);

	if(get_file_size(argv[1]) > 512) {
		cartridge_load_from_file(argv[1], &cart);
		WORLD_M68K(w).pc = be_to_host_glong(cart->pc);
	} else {
	}
	read_file(argv[1], MEMORY_DATA(WORLD_MEM(w)), GENESIS_MEMORY_LEN);

	cycles = 0;

	while(1) {
		m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w), &cycles);
		sleep(0);
	}

	return result;
}
