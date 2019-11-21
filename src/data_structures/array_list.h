/** @file
* @brief Variable length data structure
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#ifndef ARRAY_LIST_H
# define ARRAY_LIST_H

#include <stddef.h>

#define MAX_INIT_SIZE 64

/**
* @struct array_list
* @brief Variable length data structure
*/

struct array_list
{
    void **content; /**< @brief the array_list content */
    size_t nb_element; /**< @brief the amount of elements contained */
    size_t max_size; /**< @brief the biggest size the structure can get to */
};

/**
* @brief initializes an array_list structure with a size of MAX_INIT_SIZE
* @return array_list structure
*/
struct array_list *array_list_init(void);

/**
* @brief adds an element to an array_list structure that is resized if needed
* @brief from the command name and an array_list structure
* @param array_list structure
* @param element to add to array_list structure
*/
void array_list_append(struct array_list *, void *);

/**
* @brief destroys the array_list structure and its content
* @brief Pointer in content NEEDS to be freeable (coming from malloc)
* @param array_list structure
*/
void array_list_destroy(struct array_list *);

#endif /* ! ARRAY_LIST_H */
