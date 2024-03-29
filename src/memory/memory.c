#include <stdlib.h>
#include <err.h>

#include "memory.h"

void memory_exhausted(void)
{
    err(1, "Memory exhausted.");
}

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (size && !ptr)
        memory_exhausted();

    return ptr;
}

void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    if (size && nmemb && !ptr)
        memory_exhausted();

    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (size && new_ptr == NULL)
    {
        free(ptr);
        memory_exhausted();
    }
    return new_ptr;
}