/** @file
* @brief lexer for arithmetic expressions
* @author Coder : cloe.lacombe & pierrick.made
* @author Tester : cloe.lacombe & pierrick.made
* @author Reviewer :
*/

#pragma once
#include <stddef.h>

enum token_type{
    TOKEN_PARAM,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_NOT,
    TOKEN_BITWISE_XOR,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_NOT,
    TOKEN_UNARY_MINUS,
    TOKEN_LEFT_PARENTHESIS,
    TOKEN_RIGHT_PARENTHESIS
};

struct token
{
    enum token_type type;
    int priority;
    char *data;
};

struct token *token_create(enum token_type type, int priority, char *data);

struct token *token_get_next(char **line);

void token_free(struct token **token);

char *xstrndup(char *str, size_t n);
