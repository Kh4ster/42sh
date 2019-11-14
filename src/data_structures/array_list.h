#ifndef ARRAY_LIST_H
# define ARRAY_LIST_H

#include <stddef.h>

#define MAX_INIT_SIZE 64

struct array_list
{
    void **content;
    size_t nb_element;
    size_t max_size;
};

struct array_list* array_list_init(void);
void array_list_append(struct array_list*, void*);
void array_list_destroy(struct array_list*);

#endif /* ! ARRAY_LIST_H */