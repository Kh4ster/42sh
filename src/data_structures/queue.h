/** @file
* @brief Data_structure representing a queue (FIFO)
* @author Coder : pierrick.made
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

# include <stddef.h>

struct list
{
    void *data;
    struct list *next;
};

struct queue
{
    struct list *head;
    struct list *tail;
    size_t size;
};

struct queue *queue_init(void);
size_t queue_size(struct queue *queue);
void queue_push(struct queue *queue, void *elt);
void* queue_head(struct queue *queue);
void* queue_pop(struct queue *queue);
void queue_clear(struct queue *queue);
void queue_destroy(struct queue *queue);