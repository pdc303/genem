#ifndef __STACK_H__
#define __STACK_H__

#include <stddef.h>
#include "types.h"

struct stack
{
	/* number of items allocated to the stack */
	size_t stack_size;
	/* number of items on the stack */
	size_t num_items;
	/* virtual (genesis) address of the stack */
	glong stack_virtual_address;
	glong stack_pointer;
	glong data[];
};

void stack_init(struct stack *st, size_t stack_size, glong vaddr);
void stack_finish(struct stack *st);
void stack_push(struct stack *st, glong val);
glong stack_pop(struct stack *st);

#endif
