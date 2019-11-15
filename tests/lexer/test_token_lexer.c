#include <criterion/criterion.h>
#include <string.h>

#include "../../src/lexer/token_lexer.h"
#include "../../src/data_structures/queue.h"
#include "../../src/data_structures/data_string.h"

Test(token_lexer, lexer_simple)
{
    struct string *need_lexing = string_init();
    string_append(need_lexing, "echo test");
    struct queue *token_queue = lexer(need_lexing->content, NULL);

    struct token_lexer *current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "echo"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "test"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    string_free(&need_lexing);
    queue_destroy(&token_queue);
}

Test(token_lexer, lexer_simple_and_newline)
{
    struct string *need_lexing = string_init();
    string_append(need_lexing, "echo test\n");
    struct queue *token_queue = lexer(need_lexing->content, NULL);

    struct token_lexer *current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "echo"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "test"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "\n"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    string_free(&need_lexing);
    queue_destroy(&token_queue);
}

Test(token_lexer, lexer_less_simple_multiple_tokens)
{
    struct string *need_lexing = string_init();
    string_append(need_lexing, "echo test\nif ...; then wow;fi");
    struct queue *token_queue = lexer(need_lexing->content, NULL);

    struct token_lexer *current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "echo"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "test"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "\n"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "if"));
    cr_assert_eq(TOKEN_KEYWORD, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "..."));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, ";"));
    cr_assert_eq(TOKEN_END_OF_INSTRUCTION, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "then"));
    cr_assert_eq(TOKEN_KEYWORD, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "wow"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, ";"));
    cr_assert_eq(TOKEN_END_OF_INSTRUCTION, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "fi"));
    cr_assert_eq(TOKEN_KEYWORD, current_token->type);
    token_lexer_free(&current_token);

    string_free(&need_lexing);
    queue_destroy(&token_queue);
}

Test(token_lexer, lexer_operators_simple)
{
    struct string *need_lexing = string_init();
    string_append(need_lexing, "hey&&you&&&;");
    struct queue *token_queue = lexer(need_lexing->content, NULL);

    struct token_lexer *current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "hey"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "&&"));
    cr_assert_eq(TOKEN_OPERATOR, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "you"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "&&"));
    cr_assert_eq(TOKEN_OPERATOR, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, "&"));
    cr_assert_eq(TOKEN_OTHER, current_token->type);
    token_lexer_free(&current_token);

    current_token = queue_pop(token_queue);
    cr_assert_not_null(current_token);
    cr_assert_eq(0, strcmp(current_token->data, ";"));
    cr_assert_eq(TOKEN_END_OF_INSTRUCTION, current_token->type);
    token_lexer_free(&current_token);

    string_free(&need_lexing);
    queue_destroy(&token_queue);
}
