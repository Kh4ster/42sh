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

struct array_list* init_array_list(void);
void append_array_list(struct array_list*, void*);
void destroy_array_list(struct array_list*);

#endif /* ! ARRAY_LIST_H */