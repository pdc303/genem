#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "cartridge.h"
#include "memory.h"
#include "sysmem.h"
#include "sysutil.h"

int cartridge_load_from_file(const char *filename, struct cartridge **cart)
{
	FILE *fp;
	int n, result;
	size_t leftover, filesize;

	result = 0;
	*cart = NULL;
	fp = NULL;

	fp = fopen(filename, "r");
	
	if(fp == NULL) {
		return -EIO;
		goto cartridge_load_from_file_out;
	}
	
	filesize = get_file_size(filename);
	leftover = CARTRIDGE_LEFTOVER(filesize);

	*cart = genem_malloc(filesize);

	n = fread(*cart, 1, filesize, fp);

	if(n != filesize) {
		result = -EIO;
		goto cartridge_load_from_file_out;
	}

cartridge_load_from_file_out:
	if(fp != NULL) {
		fclose(fp);
	}
	if((result != 0) && (*cart != NULL)) {
		genem_free(*cart);
		*cart = NULL;
	}

	return result;
}
