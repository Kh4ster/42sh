/** @file
* @brief lexer for arithmetic expressions
* @author Coder : cloe.lacombe & pierrick.made
* @author Tester : cloe.lacombe & pierrick.made
* @author Reviewer :
*/

#pragma once
#include <stddef.h>

enum token_type{
    AR_TOKEN_PARAM,
    AR_TOKEN_AND,
    AR_TOKEN_OR,
    AR_TOKEN_BITWISE_AND,
    AR_TOKEN_BITWISE_OR,
    AR_TOKEN_BITWISE_NOT,
    AR_TOKEN_BITWISE_XOR,
    AR_TOKEN_PLUS,
    AR_TOKEN_MINUS,
    AR_TOKEN_MULTIPLY,
    AR_TOKEN_DIVIDE,
    AR_TOKEN_POWER,
    AR_TOKEN_NOT,
    AR_TOKEN_UNARY_MINUS,
    AR_TOKEN_UNARY_PLUS,
    AR_TOKEN_LEFT_PARENTHESIS,
    AR_TOKEN_RIGHT_PARENTHESIS
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
