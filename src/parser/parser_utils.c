#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <err.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"
#include "ast_build.h"
#include "ast/ast.h"
#include "ast/destroy_tree.h"
#include "../lexer/token_lexer.h"
#include "../memory/memory.h"
#include "../execution_handling/command_container.h"
#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"
#include "../redirections_handling/redirect.h"
#include "../data_structures/data_string.h"
#include "parser_utils.h"

#define NEXT_IS(X) next_is(lexer, X)
#define EAT() eat_token(lexer)
#define NEXT_IS_EOF() next_is_eof(lexer)
#define NEXT_IS_OTHER() next_is_other(lexer)
#define NEXT_IS_ASSIGNEMENT() next_is_assignement(lexer)
#define NEXT_IS_NUMBER() next_is_number(lexer)

bool is_function(struct queue *lexer)
{
    if (NEXT_IS("function"))
        return true;
    else if (next_next_is(lexer, "("))
        return true;
    return false;
}

enum token_parser_type token_is_redirection (struct token_lexer *token)
{
    enum token_parser_type type = 0;

    if (!strcmp(token->data, ">"))
        type = TOKEN_REDIRECT_LEFT;

    if (!strcmp(token->data, "<"))
        type = TOKEN_REDIRECT_RIGHT;

    if (!strcmp(token->data, ">>"))
        type = TOKEN_REDIRECT_APPEND_LEFT;

    if (!strcmp(token->data, ">&"))
        type = TOKEN_REDIRECT_LEFT_TO_FD;

    if (!strcmp(token->data, "<>"))
        type = TOKEN_REDIRECT_READ_WRITE;

    if (!strcmp(token->data, "<&"))
        type = TOKEN_DUP_FD;

    if (!strcmp(token->data, ">|"))
        type = TOKEN_OVERWRITE;

    if (!strcmp(token->data, "<<"))
        type = TOKEN_HEREDOC;

    if (!strcmp(token->data, "<<-"))
        type = TOKEN_HEREDOC_MINUS;

    return type;
}

enum token_parser_type is_redirection(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);

    return token_is_redirection(token);
}

bool is_shell_command(struct queue *lexer)
{
    static const char *shell_command[] =
    {
        "if",
        "for",
        "while",
        "until",
        "case",
        "{",
        "("
    };
    size_t size_array = sizeof(shell_command) / sizeof(char *);

    for (size_t i = 0; i < size_array; ++i)
    {
        if (NEXT_IS(shell_command[i]))
            return true;
    }
    return false;
}

bool next_is_assignement(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_ASSIGNEMENT;
}

bool next_is_other(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_OTHER;
}

bool next_is_number(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);

    if (token == NULL)
        return false;
    return token->type == TOKEN_IO_NUMBER;
}

bool next_is_control_structure(struct queue *lexer)
{
    static const char *shell_command[] =
    {
        "if",
        "for",
        "while",
        "until",
        "case"
    };
    size_t size_array = sizeof(shell_command) / sizeof(char *);

    for (size_t i = 0; i < size_array; ++i)
    {
        if (NEXT_IS(shell_command[i]))
            return true;
    }
    return false;
}

int next_is_end_of_case_item(struct queue *lexer)
{
    return NEXT_IS(";;") || NEXT_IS("(") || NEXT_IS(")") || NEXT_IS("esac");
}

bool next_is_end_of_instruction(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return true;

    if (strcmp(token->data, "\n") == 0)
        return true;

    if (NEXT_IS("(") || NEXT_IS(")"))
        return true;

    return token->type == TOKEN_END_OF_INSTRUCTION
            || token->type == TOKEN_EOF
            || (token->type == TOKEN_OPERATOR && !is_redirection(lexer));
}

bool next_is_eof(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_EOF;
}

bool next_is(struct queue *lexer, const char *to_match)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return strcmp(token->data, to_match) == 0;
}

bool next_next_is(struct queue *lexer, const char *to_match)
{
    struct token_lexer *next_next = lexer_next_next(lexer);
    if (next_next == NULL)
        return false;
    return strcmp(next_next->data, to_match) == 0;
}