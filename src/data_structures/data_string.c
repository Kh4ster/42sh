/*
** Coder : nicolas.blin
** Tester : 
** Reviewer : 
** Integrator : 
**
** Can hold a variable lenth array of char
*/

#include <stdlib.h>
#include <string.h>

#include "data_string.h"
#include "../memory/memory.h"

#define BASE_STR_CAPACITY 64

/*
** init an empty, need to be freed later
*/
struct string* init_string(void)
{
    struct string *s = my_malloc(sizeof(struct string));
    s->capacity = BASE_STR_CAPACITY;
    s->index = 0;
    s->content = my_calloc(BASE_STR_CAPACITY, sizeof(char));
    return s;
}

/*
** empty string content without freeing it
*/
void empty_string_content(struct string *string)
{
    assert(string != NULL);
    assert(string->content != NULL);

    for (size_t i = 0; i < string->capacity; i++)
        string->content[i] = 0;
    string->index = 0;
}

/*
** append to_happend at the end of str
** if needed, size will be
*/
void append_string(struct string *str, char *to_append)
{
    assert(str != NULL);
    assert(str->content != NULL);
    assert(to_append != NULL);

    size_t new_size = strlen(to_append) + str->index;
    if (new_size >= str->capacity)
    {
        str->capacity *= 2;
        str->content = xrealloc(str->content, str->capacity);
        append_string(str, to_append); //recur call if new size still to small
    }
    else
    {
        str->index += strlen(to_append);
        strcat(str->content, to_append);
    }
}

/*
** appends only n bytes to string
*/
void append_n_string(struct string *str, const char *to_append, size_t n)
{
    assert(str != NULL);
    assert(str->content != NULL);
    assert(to_append != NULL);

    size_t new_size = n + str->index;
    if (new_size >= str->capacity)
    {
        str->capacity *= 2;
        str->content = xrealloc(str->content, str->capacity);
        append_n_string(str, to_append, n);
    }
    else
    {
        str->index += strlen(to_append);
        strncat(str->content, to_append, n);
    }
}

/*
** appends one char to the string
*/
void append_char_string(struct string *str, char c)
{
    assert(str != NULL);

    char arr[2];
    arr[0] = c;
    arr[1] = 0;
    append_string(str, arr);
}

/*
** free string except it's content and returns it
*/
char *get_content_string(struct string *str)
{
    assert(str != NULL);

    char *content = str->content;
    str->content = NULL;
    free_string(str);
    return content;
}

/*
** free string completly
*/
void free_string(struct string *str)
{
    assert(str != NULL);

    char *content = str->content;
    str->content = NULL;
    free(content);
    str->capacity = 0;
    str->index = 0;
    free(str);
}