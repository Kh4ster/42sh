#include <criterion/criterion.h>
#include <string.h>
#include <stdlib.h>

#include "../../src/data_structures/data_string.h"

Test(string, string_init)
{
    struct string *str = string_init();
    cr_assert_not_null(str);
    cr_assert_eq(BASE_STR_CAPACITY, str->capacity);
    cr_assert_eq(0, str->index);
    cr_assert_not_null(str->content);
    string_free(&str);
}

Test(string, basic_append)
{
    struct string *str = string_init();
    string_append(str, "Hello ");
    cr_assert_eq(0, strcmp(str->content, "Hello "));
    string_append(str, "World !");
    cr_assert_eq(0, strcmp(str->content, "Hello World !"));
    string_free(&str);
}

Test(string, append_greater_base_capacity)
{
    struct string *str = string_init();
    char *test = malloc(BASE_STR_CAPACITY + 5);
    for (size_t i = 0; i < BASE_STR_CAPACITY + 4; i++)
        test[i] = 'a';
    test[str->capacity + 4] = 0;
    string_append(str, test);
    cr_assert_eq(BASE_STR_CAPACITY * 2, str->capacity);
    for (size_t i = 0; i < BASE_STR_CAPACITY + 5; i++)
        cr_assert_eq(test[i], str->content[i]);
    string_free(&str);
    free(test);
}

Test(string, append_much_greater_base_capacity)
{
    struct string *str = string_init();
    char *test = malloc(BASE_STR_CAPACITY * 5);
    for (size_t i = 0; i < BASE_STR_CAPACITY * 5 - 1; i++)
        test[i] = 'a';
    test[str->capacity * 5 - 1] = 0;
    string_append(str, test);
    cr_assert_eq((BASE_STR_CAPACITY << 3), str->capacity);
    for (size_t i = 0; i < BASE_STR_CAPACITY * 5 - 1; i++)
        cr_assert_eq(test[i], str->content[i]);
    string_free(&str);
    free(test);
}

Test(string, basic_n_append)
{
    struct string *str = string_init();
    string_append_n(str, "Hello @@@", 6);
    cr_assert_eq(0, strcmp(str->content, "Hello "));
    string_append_n(str, "World !@@@@", 7);
    cr_assert_eq(0, strcmp(str->content, "Hello World !"));
    string_free(&str);
}

Test(string, basic_append_char)
{
    struct string *str = string_init();
    for (size_t i = 0; i < 4; i++)
        string_append_char(str, 'a');
    cr_assert_eq(0, strcmp(str->content, "aaaa"));
    string_free(&str);
}

Test(string, string_get_string_content)
{
    struct string *str = string_init();
    string_append(str, "Hello");
    char *content = string_get_content(&str);
    cr_assert_eq(0, strcmp(content, "Hello"));
    cr_assert_null(str);
    free(content);
}

Test(string, string_empty_content)
{
    struct string *str = string_init();
    string_append(str, "Hello");
    string_empty_content(str);
    cr_assert_eq(0, str->index);
    for (size_t i = 0; i < str->capacity; ++i)
        cr_assert_eq(0, str->content[i]);
    string_free(&str);
}

Test(string, string_free)
{
    struct string *str = string_init();
    string_append(str, "Hello");
    string_free(&str);
    cr_assert_null(str);
}