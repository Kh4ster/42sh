#ifndef DATA_STRING_H
# define DATA_STRING_H

#include <stddef.h>

#define BASE_STR_CAPACITY 64

struct string
{
    char *content;
    size_t index;
    size_t capacity;
};

struct string* string_init(void);
void string_append(struct string*, char*);
void string_free(struct string**);
void string_append_n(struct string*, const char*, size_t);
void string_append_char(struct string*, char);
char* string_get_content(struct string**);
void string_empty_content(struct string*);

#endif /* ! DATA_STRING_H */