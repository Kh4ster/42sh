#include <assert.h>
#include <stdlib.h>

#include "queue.h"
#include "../memory/memory.h"

struct queue *queue_init(void)
{
    struct queue *new_queue = xmalloc(sizeof(struct queue));

    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->size = 0;
    return new_queue;
}

void queue_push(struct queue *queue, void *elt)
{
    if (queue == NULL)
        queue = queue_init();
    struct queue_item *new_l = xmalloc(sizeof(struct queue_item));

    new_l->data = elt;
    new_l->next = NULL;
    if (queue->tail != NULL)
        queue->tail->next = new_l;
    queue->tail = new_l;
    if (queue->size == 0)
        queue->head = new_l;
    queue->size += 1;
}

void queue_push_start(struct queue *queue, void *elt)
{
    if (queue == NULL)
        queue = queue_init();
    struct queue_item *new_l = xmalloc(sizeof(struct queue_item));

    new_l->data = elt;
    new_l->next = queue->head;
    queue->head = new_l;
    queue->size += 1;
}

void *queue_head(struct queue *queue)
{
    assert(queue != NULL);
    if (queue->head == NULL)
        return NULL;
    return queue->head->data;
}

void *queue_pop(struct queue *queue)
{
    assert(queue != NULL);

    if (queue->head == NULL)
        return NULL;

    void *elt_popped = queue->head->data;
    if (queue->head == queue->tail)
    { // last item
        free(queue->head);
        queue->tail = NULL;
        queue->head = NULL;
        queue->size -= 1;
        return elt_popped;
    }
    struct queue_item *tmp = queue->head->next;
    free(queue->head);
    queue->head = tmp;
    queue->size -= 1;
    return elt_popped;
}

void queue_clear(struct queue *queue)
{
    assert(queue != NULL);
    while (queue->head != NULL)
    {
        void *popped = queue_pop(queue);
        free(popped);
    }
}

void queue_destroy(struct queue **queue)
{
    assert(queue != NULL);
    struct queue *q = *queue;
    queue_clear(q);
    free(q);
    *queue = NULL;
}
