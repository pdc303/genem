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

#include "time.h"

int main(int argc, char **argv)
{
	int result;
	struct world w;
	struct cartridge *cart;
	int cycles;
	int op, real;
	int i, ta, tb, tt;
	float mhz;

	world_init(&w);

	if(get_file_size(argv[1]) > 512) {
		cartridge_load_from_file(argv[1], &cart);
		WORLD_M68K(w).pc = be_to_host_glong(cart->pc);
	} else {
	}
	read_file(argv[1], MEMORY_DATA(WORLD_MEM(w)), GENESIS_MEMORY_LEN);

	cycles = 0;

	#define CNT 200000000
	i = CNT;
	ta = time(NULL);
	while(i--) {
		gclock_t cycles = WORLD_M68K(w).cycles;
		m68000_exec(WORLD_PM68K(w), WORLD_PMEM(w));
		cycles = WORLD_M68K(w).cycles - cycles;
		if(cycles == 0) {
			dbg_w("Instruction used no cycles");
		}
		//dbg_i("Cycles: %" PRIu64, cycles);
	}
	tb = time(NULL);

	tt = tb - ta;
	mhz = ((WORLD_M68K(w).cycles) / tt) / 1000000;

	dbg_i("%"PRIu64 " clock cycles in %u seconds = %.2f MHz",
				WORLD_M68K(w).cycles,
				tt,
				mhz,
				WORLD_M68K(w).cycles/(tt));

	return result;
}
