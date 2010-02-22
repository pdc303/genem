#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__
#include <stdint.h>

#include "types.h"

/* calculate how many bytes are in a cartridge apart from the header */
#define CARTRIDGE_LEFTOVER(filesize) (filesize - sizeof(struct cartridge) + 1)

struct cartridge
{
	/* interrupt vectors */
	/* 68000 interrupts */
	glong sp;
	glong pc; /* start of program in ROM after reset */
	glong buserr;
	glong addrerr;
	glong illegalinst;
	glong divzero;
	glong chk;
	glong trapv;
	glong privviol;
	glong trace;
	/* external interrupts */
	glong linea;
	glong linef;
	glong unused1[12];
	glong spurious;
	glong irq1;
	glong irq2;
	glong irq3;
	glong irq4; /* HBlankInterrupt */
	glong irq5;
	glong irq6; /* VBlankInterrupt */
	glong irq7;
	glong trapex[16];
	glong unused2[16];
	/* end of interrupt vectors */
	char system_name[16];
	char copyright[16];
	char game_name_domestic[48];
	char game_name_overseas[48];
	char type[14];
	byte checksum[2];
	char control_data[16];
	byte rom_capacity[8]; // 4 bytes start address, 4 bytes end address
	byte ram_capacity[8]; // 4 bytes start address, 4 bytes end address
	byte external_ram_data[12];
	byte modem_data[12];
	char memo[40];
	char country[16];
	byte data[]; // all the data which follows this header
};

int cartridge_load_from_file(const char *filename, struct cartridge **cart);

#endif /* __CARTRIDGE_H__ */
