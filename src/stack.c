#include "stack.h"

#include "types.h"
#include "memory.h"
#include "sysmem.h"
#include "debug.h"

/* vaddr shall me the location in virtual memory where the stack structure begins. */
void stack_init(struct stack *st, size_t stack_size, glong vaddr)
{
	int i;

	st->stack_size = stack_size;
	st->stack_virtual_address = vaddr + (sizeof(struct stack));
	/* ptr to the next available slot */
	st->stack_pointer = st->stack_virtual_address + (sizeof(glong) * (stack_size - 1));

	st->num_items = 0;

	for(i = 0; i < stack_size; i++) {
		st->data[i] = 0;
	}
}

void stack_finish(struct stack *st)
{
	st->num_items = 0;
}

void stack_push(struct stack *st, glong val)
{
	if(st->num_items == st->stack_size) {
		dbg_f("Stack overflow");
	}

	val = host_to_be_glong(val);

	st->data[st->stack_size - st->num_items - 1] = val;
	st->num_items++;
	st->stack_pointer -= sizeof(glong);
}

glong stack_pop(struct stack *st)
{
	glong val;

	if(st->num_items == 0) {
		dbg_f("Stack underflow");
		return 0;
	}

	st->num_items--;
	st->stack_pointer += sizeof(glong);
	val = st->data[st->stack_size - st->num_items - 1];
	
	val = be_to_host_glong(val);

	return val;
}
