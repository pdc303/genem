#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include "sysutil.h"
#include "types.h"
#include "debug.h"
#include "config.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

size_t get_file_size(const char *filename)
{
#ifdef HAVE_SYS_STAT_H
	int error;
	struct stat st;

	error = stat(filename, &st);
	if(error != 0) {
		return -EIO;
	} else {
		return st.st_size;
	}
#else
	NOFUNC();
#endif
}

size_t read_file(const char *filename, byte *buf, size_t max_len)
{
	FILE *fp;
	size_t n;

	fp = fopen(filename, "r");

	if(fp == NULL) {
		return -EIO;
	}

	n = fread(buf, 1, max_len, fp);

	fclose(fp);

	return n;
}

unsigned long now_usec()
{
#ifdef OS_LINUX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec + (tv.tv_sec * 1000000);
#else
	NOFUNC();
#endif

}
