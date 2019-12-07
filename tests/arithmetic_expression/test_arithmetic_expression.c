#include <criterion/criterion.h>
#include <string.h>

#include "../../src/arithmetic_expression/lexer.h"
#include "../../src/arithmetic_expression/parser.h"
#include "../../src/arithmetic_expression/tree.h"

Test(Lexer, token_create)
{
    struct token *new_token = token_create(TOKEN_PLUS, 2, "+");
    cr_assert_str_eq(new_token->data, "+");
    cr_assert_eq(new_token->priority, 2);
    cr_assert_eq(new_token->type, TOKEN_PLUS);
    free(new_token);
}

Test(Lexer, token_get_next_simple)
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
    cr_assert_eq(new_token->priority, 4);
    cr_assert_eq(new_token->type, TOKEN_PLUS);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "2");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    free(example);
}


Test(Lexer, token_get_next_less_simple)
{
    char *example = strdup("  3+   \t\n\t -242/\t&&4$3~ () ");
    char *cursor = example;

    struct token *new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "3");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "+");
    cr_assert_eq(new_token->priority, 4);
    cr_assert_eq(new_token->type, TOKEN_PLUS);
    token_free(&new_token);


    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "-");
    cr_assert_eq(new_token->priority, 4);
    cr_assert_eq(new_token->type, TOKEN_MINUS);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "242");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "/");
    cr_assert_eq(new_token->priority, 5);
    cr_assert_eq(new_token->type, TOKEN_DIVIDE);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "&&");
    cr_assert_eq(new_token->priority, 0);
    cr_assert_eq(new_token->type, TOKEN_AND);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "4$3");
    cr_assert_eq(new_token->priority, -1);
    cr_assert_eq(new_token->type, TOKEN_PARAM);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "~");
    cr_assert_eq(new_token->priority, 7);
    cr_assert_eq(new_token->type, TOKEN_BITWISE_NOT);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, "(");
    cr_assert_eq(new_token->priority, -2);
    cr_assert_eq(new_token->type, TOKEN_LEFT_PARENTHESIS);
    token_free(&new_token);

    new_token = token_get_next(&cursor);
    cr_assert_str_eq(new_token->data, ")");
    cr_assert_eq(new_token->priority, -2);
    cr_assert_eq(new_token->type, TOKEN_RIGHT_PARENTHESIS);
    token_free(&new_token);

    free(example);
}


Test (Tree, evaluate_tree)
{
    struct node *out = parser("3 + 1");
    cr_assert_eq(4, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("3 + 1 * 2");
    cr_assert_eq(5, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("3 * 2 ** 2");
    cr_assert_eq(12, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("(3 * 2) ** 2");
    cr_assert_eq(36, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("3 && 0 || 2");
    cr_assert_eq(1, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("3 || 0 && 0");
    cr_assert_eq(1, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("3 & 1 | 2");
    cr_assert_eq(3, evaluate_tree(out));

    //destroy_tree(out);
    out = parser("(3 + 2) * -(2 + 3)");
    cr_assert_eq(-25, evaluate_tree(out));

    //destroy_tree(out);
}
