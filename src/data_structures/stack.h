/**
* @brief Generic stack
* @author Coder : cloe.lacombe
* @author reviewer : pierrick.made
*/

#ifndef STACK_H
#define STACK_H

#include <stddef.h>

/** @struct node
* @brief One node of a stack containing its value and a pointer to the next node
*/
struct stack_node
{
    void *data;
    struct stack_node *next;
};


/** @struct stack
* @brief Wrapper around the nodes of a stack that holds the head of a stack and
its size
*/
struct stack
{
    struct stack_node *head;
    size_t size;
};

/**
* @brief Created an empty stack and returns it
* @return empty stack on success, errx otherwise
* @related stack
**/
struct stack *init_stack(void);

/**
* @brief function to push an element on top of the stack.
* @param stack : stack on which to push. element: element to push
* @return same stack with element as the new head on success, errx otherwise
* @related stack
**/
struct stack *stack_push(struct stack *stack, void *element);

/**
* @brief function to pop and return the head of a stack
* @param stack : stack on which to pop
* @return previous head of the stack on success, NULL otherwise
* @related stack
**/
void *stack_pop(struct stack *stack);

/**
* @brief function to get the data on top of the stack
* @param stack : stack of which we want the head's data
* @return the data of the head of the stack on success, NULL otherwise
* @related stack
**/
void *stack_peek(struct stack *stack);

/**
* @brief function to return the size of a stack.
* @param stack : the stack of which we want the size
* @return the size of the stack on success, else -1
* @related stack
**/
size_t size_stack(struct stack *stack);

#endif
