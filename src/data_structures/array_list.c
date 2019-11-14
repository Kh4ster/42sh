#include <stdlib.h>
#include <assert.h>

#include "array_list.h"
#include "../memory/memory.h"

/*
** Init an empty array_list with a beginning size of MAX_INIT_SIZE
** Holds an array of void*
*/
struct array_list* array_list_init(void)
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
void array_list_append(struct array_list *l, void *ptr)
{
    assert(l != NULL);
    assert(l->content != NULL);
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
void array_list_destroy(struct array_list *list)
{
    assert(list != NULL);
    assert(list->content != NULL);


    for (size_t i = 0; i < list->nb_element; i++)
        free(list->content[i]);
    void **content = list->content;
    list->content = NULL;
    free(content);
    list->nb_element = -1;
    list->max_size = 0;
    free(list);
}
