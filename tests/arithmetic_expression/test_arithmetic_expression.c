#include <criterion/criterion.h>
#include <string.h>

#include "../../src/arithmetic_expression/lexer.h"

Test(Lexer, token_create)
{
    struct token *new_token = token_create(TOKEN_PLUS, 2, "+");
    cr_assert_str_eq(new_token->data, "+");
    cr_assert_eq(new_token->priority, 2);
    cr_assert_eq(new_token->type, TOKEN_PLUS);
    free(new_token);
}

Test(Lexer, token_get_next)
{
    char *example = strdup("3 + 2");
    char *cursor = example;

    struct token *new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "3");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "+");
    cr_assert_eq(new_token->priority, 2);
    cr_assert_eq(new_token->type, TOKEN_PLUS);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "2");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    free(example);
}