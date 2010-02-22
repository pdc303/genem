#ifndef __SYSMEM_H__
#define __SYSMEM_H__

void *genem_malloc(size_t size);
void *genem_zalloc(size_t size);
void genem_free(void *ptr);

#endif /* __SYSMEM_H__ */
