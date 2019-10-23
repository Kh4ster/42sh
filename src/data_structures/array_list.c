/*
** Coder : nicolas.blin
** Tester : 
** Reviewer : 
** Integrator : 
**
** Variable length array data_structure
** TO TEST IF NEEDED BY THE PROJECT
*/

#include <stdlib.h>
#include <assert.h>

#include "array_list.h"
#include "memory.h"

#define MAX_INIT_SIZE 64

/*
** Init an empty array_list with a beginning size of MAX_INIT_SIZE
** Holds an array of void*
*/
struct array_list* init_array_list(void)
{
    struct array_list *arr = xmalloc(sizeof(struct array_list));
    arr->max_size = MAX_INIT_SIZE;
    arr->nb_element = 0;
    arr->content = xcalloc(MAX_INIT_SIZE, sizeof(void*));
    return arr;
}


/*
** Append an element at the end of the array_list
** If it needs, will be resized
*/
void append_array_list(struct array_list *l, void *ptr)
{
    assert(l != NULL);
    assert(ptr != NULL);
    if (l->nb_element + 1 > l->max_size)
    {
        l->max_size *= 2;
        l->content = xrealloc(l->content, sizeof(void*) * l->max_size);
    }
    l->content[l->nb_element++] = ptr;
}

/*
** Detroy the array_list and its content
** Pointer in content NEED to be freeable (coming from malloc)
*/
void destroy_array_list(struct array_list *list)
{
    assert(list != NULL);
    for (size_t i = 0; i < list->nb_element; i++)
        free(list->content[i]);
    free(list->content);
    free(list);
}