#include <criterion/criterion.h>
#include <string.h>
#include <stdlib.h>

#include "../../src/data_structures/data_string.h"

Test(string, initstring)
{
    struct string *str = init_string();
    cr_assert_not_null(str);
    cr_assert_eq(BASE_STR_CAPACITY, str->capacity);
    cr_assert_eq(0, str->index);
    cr_assert_not_null(str->content);
}

Test(string, basic_append)
{
    struct string *str = init_string();
    append_string(str, "Hello ");
    cr_assert_eq(0, strcmp(str->content, "Hello "));
    append_string(str, "World !");
    cr_assert_eq(0, strcmp(str->content, "Hello World !"));
}

Test(string, append_greater_base_capacity)
{
    struct string *str = init_string();
    char *test = malloc(BASE_STR_CAPACITY + 5);
    for (size_t i = 0; i < BASE_STR_CAPACITY + 4; i++)
        test[i] = 'a';
    test[str->capacity + 4] = 0;
    append_string(str, test);
    cr_assert_eq(BASE_STR_CAPACITY * 2, str->capacity);
    for (size_t i = 0; i < BASE_STR_CAPACITY + 5; i++)
        cr_assert_eq(test[i], str->content[i]);
}

Test(string, basic_n_append)
{
    struct string *str = init_string();
    append_n_string(str, "Hello @@@", 6);
    cr_assert_eq(0, strcmp(str->content, "Hello "));
    append_n_string(str, "World !@@@@", 7);
    cr_assert_eq(0, strcmp(str->content, "Hello World !"));
}

Test(string, basic_append_char)
{
    struct string *str = init_string();
    for (size_t i = 0; i < 4; i++)
        append_char_string(str, 'a');
    cr_assert_eq(0, strcmp(str->content, "aaaa"));
}

Test(string, string_get_string_content)
{
    struct string *str = init_string();
    append_string(str, "Hello");
    char *content = get_content_string(&str);
    cr_assert_eq(0, strcmp(content, "Hello"));
    cr_assert_null(str);
}

Test(string, empty_string_content)
{
    struct string *str = init_string();
    append_string(str, "Hello");
    empty_string_content(str);
    cr_assert_eq(0, str->index);
    for (size_t i = 0; i < str->capacity; ++i)
        str->content[i] = 0;
}

Test(string, free_string)
{
    struct string *str = init_string();
    append_string(str, "Hello");
    free_string(&str);
    cr_assert_null(str);
}