#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#include "../../src/data_structures/stack.h"


void test_pop()
{
    struct stack *s = init_stack();
    size_t args[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    for (int i = 0; i < 10; i ++)
        s = stack_push(s, &args[i]);

    size_t *test = stack_pop(s);
    assert( *test == 10);
    assert(s->size == 9);
    assert(s->head->data == &args[8]);

    for (size_t i = 1; i <= 9; i++)
    {
        test = stack_pop(s);
        assert(*test == 9 - (i - 1));
        assert(s->size == 9 - i);
    }

    test = stack_pop(s);
    assert(test == NULL);
    assert(s->size == 0);
}

void test_push()
{
    struct stack *s = init_stack();
    int a = 12;
    s = stack_push(s, &a);
    assert(s->size == 1);
    assert(s->head);
    assert(!s->head->next);
    assert(s->head->data == &a);
    char *test_char_star = "12345678";
    s = stack_push(s, test_char_star);
    assert(s->size == 2);
    assert(s->head);
    assert(!s->head->next->next);
    assert(s->head->next->data == &a);
    assert(s->head->data == test_char_star);
    struct node *ex_head = s->head;
    s = stack_push(s, NULL);
    assert(s->size == 2);
    assert(s->head == ex_head);
}


void test_init()
{
    struct stack *s = init_stack();
    assert(s);
    assert(!s->head);
    assert(!s->size);
}


int main(void)
{
    test_init();
    test_push();
    puts("All test completed succesfully");
}
