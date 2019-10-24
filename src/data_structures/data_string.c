/*
** Coder : nicolas.blin
** Tester : nicolas.blin
** Reviewer : pierrick.made
** Integrator :
**
** Can hold a variable length array of char
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "data_string.h"
#include "../memory/memory.h"

/*
** init an empty struct string, need to be freed later
*/
struct string* init_string(void)
{
    struct string *s = xcalloc(1, sizeof(struct string));
    s->capacity = BASE_STR_CAPACITY;
    s->index = 0;
    s->content = xcalloc(BASE_STR_CAPACITY, sizeof(char));
    return s;
}

/*
** empty string content without freeing it
*/
void empty_string_content(struct string *string)
{
    assert(string != NULL);
    assert(string->content != NULL);

    memset(string->content, 0, string->capacity);
    string->index = 0;
}

/*
** append to_happend at the end of str
** if needed, size will be ^2 more
*/
void append_string(struct string *str, char *to_append)
{
    assert(str != NULL);
    assert(str->content != NULL);
    assert(to_append != NULL);

    size_t new_size = strlen(to_append) + str->index;
    if (new_size >= str->capacity)
    {
        while (new_size >= str->capacity)
            str->capacity <<= 1;
        str->content = xrealloc(str->content, str->capacity);
    }
    str->index += strlen(to_append);
    strcat(str->content, to_append);
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
        while (new_size >= str->capacity)
            str->capacity <<= 1;
        str->content = xrealloc(str->content, str->capacity);
    }

    str->index += strlen(to_append);
    strncat(str->content, to_append, n);
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
char* get_content_string(struct string **ptr_str)
{
    assert(ptr_str != NULL);
    struct string *str = *ptr_str;
    assert(str != NULL);

    char *content = str->content;
    str->content = NULL;
    free_string(ptr_str);
    return content;
}

/*
** free string completely
** we use a ** to make the pointer point to null to be sure not used after
*/
void free_string(struct string **ptr_str)
{
    assert(ptr_str != NULL);
    struct string *str = *ptr_str;
    assert(str != NULL);

    free(str->content);
    free(str);
    *ptr_str = NULL;
}