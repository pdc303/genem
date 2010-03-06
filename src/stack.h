#ifndef __STACK_H__
#define __STACK_H__

#include <stddef.h>

struct stack
{
	/* size of the stack members */
	size_t member_size;
	/* number of bytes allocated to the stack */
	size_t stack_size;
	void *data;
	void *stack_ptr;
	
	int nmemb;
	int nmax;
};

void stack_init(struct stack *st, size_t stack_size, size_t member_size);
void stack_finish(struct stack *st);
void stack_push(struct stack *st, void *d);
void *stack_pop(struct stack *st);

#endif
