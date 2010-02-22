#include <stdlib.h>

void *genem_malloc(size_t size)
{
	return malloc(size);
}

void *genem_zalloc(size_t size)
{
	return calloc(size, 1);
}

void genem_free(void *ptr)
{
	free(ptr);
}
