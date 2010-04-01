#include <stdio.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "m68000.h"
#include "world.h"
#include "memory.h"
#include "cartridge.h"
#include "debug.h"
#include "sysutil.h"

#include "time.h"

int main(int argc, char **argv)
{
	struct world w;
	struct cartridge *cart;

	world_init(&w);

	if(get_file_size(argv[1]) > 512) {
		cartridge_load_from_file(argv[1], &cart);
		WORLD_M68K(w).pc = be_to_host_glong(cart->pc);
	} else {
	}
	read_file(argv[1], MEMORY_DATA(WORLD_MEM(w)), GENESIS_MEMORY_LEN);

	m68000_start(&w);

	while(1);

	return 0;
}
