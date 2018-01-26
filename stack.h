#ifndef STACK_INCLUDED
#define STACK_INCLUDED

#include <stddef.h>

typedef struct stack_s * stack_t;

typedef void (*stack_apply_t)(void * item);

typedef void (*stack_foldl_t)(void * acc, void * item);


stack_t stack_new(void);

void    stack_free(stack_t stack);

size_t  stack_size(stack_t stack);

int     stack_empty(stack_t stack);

int     stack_push(stack_t stack, void const * item);

void *  stack_top(stack_t stack);

void    stack_pop(stack_t stack);

void    stack_foreach(stack_t stack, stack_apply_t f);

void    stack_foldl(stack_t stack, stack_foldl_t f, void const * acc);

#endif /* STACK_INCLUDED */
