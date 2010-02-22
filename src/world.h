#ifndef __WORLD_H__
#define __WORLD_H__

#include "m68000.h"
#include "memory.h"

/*
Input: Pointer to World
Output: Pointer to M68000
*/
#define PWORLD_PM68K(w) (&(w->m68k))
/*
Input: World
Output: Pointer to M68000
*/
#define WORLD_PM68K(w) (&(w.m68k))
/*
Input: World
Output: M68000
*/
#define WORLD_M68K(w) (w.m68k)
/*
Input: Pointer to world
Output: M68000
*/
#define PWORLD_M68K(w) (w->m68k)

/*
Input: Pointer to world
Output: Mem
*/
#define PWORLD_MEM(w) (w->mem)
/*
Input: Pointer to world
Output: Pointer to Mem
*/
#define PWORLD_PMEM(w) (&(w->mem))
/*
Input: World
Output: Mem
*/
#define WORLD_MEM(w) (w.mem)
/*
Input: World
Output: Pointer to Mem
*/
#define WORLD_PMEM(w) (&(w.mem))

struct world
{
	struct m68000 m68k;
	struct memory mem;
};

int world_init(struct world *w, size_t mem_len);

#endif /* __WORLD_H__ */
