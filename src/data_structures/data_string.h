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

struct string* init_string(void);
void append_string(struct string*, char*);
void free_string(struct string**);
void append_n_string(struct string*, const char*, size_t);
void append_char_string(struct string*, char);
char *get_content_string(struct string**);
void empty_string_content(struct string*);

#endif /* ! DATA_STRING_H */