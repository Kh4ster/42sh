/** @file
* @brief Data_structure representing a queue (FIFO)
* @author Coder : pierrick.made
* @author Tester : pierrick.made
* @author Reviewer :
* @author Integrator :
*/

#pragma once

# include <stddef.h>

/** @struct queue_item
* @brief Contains an item of the queue
*/
struct queue_item
{
    void *data; /**< @brief pointer to the data of the item */
    struct queue_item *next; /**< @brief pointer to next item of the queue */
};

/** @struct queue
* @brief Basic queue (FIFO) principle
*/
struct queue
{
    struct queue_item *head; /**< @brief pointer to first item of the queue */
    struct queue_item *tail; /**< @brief pointer to last item of the queue */
    size_t size; /**< @brief number of items in queue */
};

/**
* @brief Init a queue data structure
* @return success : initialized empty queue , fail : errx
* @relates queue
*/
struct queue *queue_init(void);

/**
* @brief Append elt pointer at the end of the queue
* @param queue Your queue struct
* @param elt pointer to the data to add at the end of queue
* @relates queue
*/
void queue_push(struct queue *queue, void *elt);

/**
* @brief Take out (pop) the first item of the queue
* @param queue Your queue struct
* @return success : pointer to the data contained in first item, fail : NULL
* @relates queue
*/
void* queue_pop(struct queue *queue);

/**
* @brief Clear queue by deleting all items
* @details DO NOT USE WITH DATA WHICH NEED RECURSIVE FREE OR NOT MALLOC-ED
* @param queue Your queue struct
* @relates queue
*/
void queue_clear(struct queue *queue);


/**
* @brief Destroy the queue struct after clearing items (queue_clear)
* @details DO NOT USE WITH DATA WHICH NEED RECURSIVE FREE OR NOT MALLOC-ED
* @param queue Your queue struct
* @relates queue
*/
void queue_destroy(struct queue **queue);