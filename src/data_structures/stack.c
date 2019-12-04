#include <stdio.h>
#include <stdlib.h>

#include "stack.h"
#include "../memory/memory.h"


struct stack *init_stack(void)
{
    struct stack *stack = xmalloc(sizeof(struct stack *));
    stack->head = NULL;
    stack->size = 0;
    return stack;
}


struct stack *stack_push(struct stack *stack, void *element)
{
    if (!element)
        return stack;

    struct stack_node *new_el = xmalloc(sizeof(struct stack_node));
    new_el->data = element;
    new_el->next = stack->head;
    stack->head = new_el;
    stack->size++;
    return stack;
}


void *stack_pop(struct stack *stack)
{
    if (!stack->head)
        return NULL;

    struct stack_node *head = stack->head;
    struct stack_node *tmp = head->next;
    void *return_value = head->data;
    free(head);
    stack->head = tmp;
    stack->size--;
    return return_value;
}


void *stack_peek(struct stack *stack)
{
    if (!stack->head)
        return NULL;

    return stack->head->data;
}


size_t size_stack(struct stack *stack)
{
    if (!stack)
        return -1;

    return stack->size;
}
