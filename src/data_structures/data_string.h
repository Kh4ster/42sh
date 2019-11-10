#ifndef DATA_STRING_H
# define DATA_STRING_H

#include <stddef.h>

#define BASE_STR_CAPACITY 64

/** @struct string
* @brief Variable length char*
*/
struct string
{
    char *content; /**< @brief text content */
    size_t index; /**< @brief current index */
    size_t capacity; /**< @brief current max capacity */
};


/**
* @brief Init a string data structure
* @return succes : initialized empty string , fail : errx
*/
struct string* string_init(void);

/**
* @brief Append text at the end of the string
* @param string Your string struct
* @param to_append char* to append at the end of string
*/
void string_append(struct string *string, const char *to_append);

/**
* @brief Free the string and its content, will set string to NULL
* @param string String to free
*/
void string_free(struct string **string);

/**
* @brief Append only the n first bytes at the end of the string
* @param string Your string struct
* @param to_append char* to append at the end of string
*/
void string_append_n(struct string *string,
                    const char *to_append,
                    size_t nb_char);

/**
* @brief Append one char at the end of the string
* @param string Your string struct
* @param c character to append at the end of string
*/
void string_append_char(struct string *string, char c);

/**
* @brief Free the string and returns its content, will set string to NULL
* @param string String to get content
*/
char* string_get_content(struct string **string);

/**
* @brief Set content to 0
* @param string String to empty
*/
void string_empty_content(struct string *string);

#endif /* ! DATA_STRING_H */