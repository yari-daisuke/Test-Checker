#include "stack.h"

#include <assert.h>
#include <stdlib.h>

struct node_s
{
    struct node_s * next;
    void          * item;
};

struct stack_s
{
    struct node_s * head;
    size_t          size;
};

stack_t stack_new(void)
{
    return calloc(1, sizeof(struct stack_s));
}

void stack_free(stack_t stack)
{
    struct node_s * iter;

    if (stack)
      {
        iter = stack->head;

        while (iter)
          {
            stack->head = iter->next;
            free(iter);
            iter = stack->head;
          }

        free(stack);
      }
}

size_t stack_size(stack_t stack)
{
    assert(stack);

    return stack->size;
}

int stack_empty(stack_t stack)
{
    assert(stack);

    return stack->size == 0;
}

int stack_push(stack_t stack, void const * item)
{
    struct node_s * tmp;

    assert(stack);

    tmp = malloc(sizeof(*tmp));

    if (!tmp)
        return 0;

    tmp->next = stack->head;
    tmp->item = (void *) item;

    stack->head = tmp;

    ++stack->size;

    return 1;
}

void * stack_top(stack_t stack)
{
    assert(stack);
    assert(stack->size > 0);

    return stack->head->item;
}

void stack_pop(stack_t stack)
{
    struct node_s * tmp;

    assert(stack);
    assert(stack->size > 0);

    tmp = stack->head;
    stack->head = tmp->next;
    free(tmp);

    --stack->size;
}

void stack_foreach(stack_t stack, stack_apply_t f)
{
    struct node_s * iter;

    assert(stack);
    assert(f);

    for (iter = stack->head; iter; iter = iter->next)
        (*f)(iter->item);
}

void stack_foldl(stack_t stack, stack_foldl_t f, void const * acc)
{
    struct node_s * iter;

    assert(stack);
    assert(f);

    for (iter = stack->head; iter; iter = iter->next)
        (*f)((void *) acc, iter->item);
}
