#include <criterion/criterion.h>

#include "../../src/memory/memory.h"
#include "../../src/data_structures/queue.h"

Test(queue, queue_init)
{
    struct queue *q = queue_init();
    cr_assert_not_null(q);
    cr_assert_null(q->head);
    cr_assert_null(q->tail);
    cr_assert_eq(q->size, 0);
    queue_destroy(&q);
}

Test(queue, queue_push_int)
{
    struct queue *q = queue_init();
    int *elt = xmalloc(sizeof(int));
    *elt = 1;
    queue_push(q, elt);
    int *first_elt_ptr = q->head->data;
    int *last_elt_ptr = q->head->data;
    cr_assert_eq(*first_elt_ptr, *elt);
    cr_assert_eq(*last_elt_ptr, *elt);
    cr_assert_eq(q->size, 1);
    queue_destroy(&q);
}

Test(queue, queue_push_ints)
{
    struct queue *q = queue_init();
    int *elt = xmalloc(sizeof(int));
    *elt = 1;
    int *elt2 = xmalloc(sizeof(int));
    *elt = 2;
    int *elt3 = xmalloc(sizeof(int));
    *elt = 3;
    queue_push(q, elt);
    queue_push(q, elt2);
    queue_push(q, elt3);
    int *first_elt_ptr = q->head->data;
    int *last_elt_ptr = q->tail->data;
    cr_assert_eq(*first_elt_ptr, *elt);
    cr_assert_eq(*last_elt_ptr, *elt3);
    cr_assert_eq(q->size, 3);
    queue_destroy(&q);
}

Test(queue, queue_pop)
{
    struct queue *q = queue_init();
    int *elt = xmalloc(sizeof(int));
    *elt = 1;
    int *elt2 = xmalloc(sizeof(int));
    *elt = 2;
    int *elt3 = xmalloc(sizeof(int));
    *elt = 3;

    queue_push(q, elt);
    queue_push(q, elt2);
    queue_push(q, elt3);
    int *popped = queue_pop(q);
    cr_assert_eq(*popped, *elt);
    free(popped);
    popped = queue_pop(q);
    cr_assert_eq(*popped, *elt2);
    free(popped);
    popped = queue_pop(q);
    cr_assert_eq(*popped, *elt3);
    free(popped);
    cr_assert_eq(q->size, 0);
    queue_destroy(&q);
}

Test(queue, queue_clear)
{
    struct queue *q = queue_init();
    int *elt = xmalloc(sizeof(int));
    *elt = 1;
    int *elt2 = xmalloc(sizeof(int));
    *elt = 2;
    int *elt3 = xmalloc(sizeof(int));
    *elt = 3;

    queue_push(q, elt);
    queue_push(q, elt2);
    queue_push(q, elt3);
    queue_clear(q);
    cr_assert_eq(q->size, 0);
    queue_destroy(&q);
}