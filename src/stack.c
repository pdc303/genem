#include "stack.h"

#include "types.h"
#include "sysmem.h"
#include "debug.h"

void stack_init(struct stack *st, size_t stack_size, size_t member_size)
{
	st->stack_size = stack_size;
	st->member_size = member_size;

	st->data = genem_malloc(stack_size);
	st->stack_ptr = ((byte *) st->data) + stack_size;

	st->nmemb = 0;
	st->nmax = stack_size / member_size;
}
void stack_finish(struct stack *st)
{
	genem_free(st->data);
	st->data = NULL;
}

void stack_push(struct stack *st, void *d)
{
	if(st->nmemb == st->nmax) {
		dbg_f("Stack full");
	}

	st->stack_ptr = ((byte *) st->stack_ptr) - st->member_size;

	switch(st->member_size) {
		case sizeof(byte):
			*((byte *) st->stack_ptr) = *((byte *) d);
			break;
		case sizeof(gword):
			*((gword *) st->stack_ptr) = *((gword *) d);
			break;
		case sizeof(glong):
			*((glong *) st->stack_ptr) = *((glong *) d);
			break;
	}

	st->nmemb++;
}

void *stack_pop(struct stack *st)
{
	void *result;

	if(st->nmemb == 0) {
		return NULL;
	}

	result = st->stack_ptr;

	st->stack_ptr = ((byte *) st->stack_ptr) + st->member_size;
	st->nmemb--;

	return result;
}
