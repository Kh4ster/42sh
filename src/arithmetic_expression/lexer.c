#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../memory/memory.h"
#include "lexer.h"

#define DELIMITERS " \t\n()+-*/|&^~!"

static void skip_blank(char **cursor)
{
    while (isspace(**cursor))
        (*cursor)++;
}


char *xstrndup(char *str, size_t n)
{
    char *copy_str = strndup(str, n);
    if (copy_str == NULL)
        memory_exhausted();
    return copy_str;
}


static int handle_and_or_power(char *line, enum token_type *data_type,
        char **data)
{
    if (strncmp(line, "&&", 2) == 0)
    {
        *data = xstrndup(line, 2);
        *data_type = TOKEN_AND;
        return 1;
    }
    else if (strncmp(line, "||", 2) == 0)
    {
        *data = xstrndup(line, 2);
        *data_type = TOKEN_OR;
        return 1;
    }
    else if (strncmp(line, "**", 2) == 0)
    {
        *data = xstrndup(line, 2);
        *data_type = TOKEN_POWER;
        return 1;
    }
    return 0;
}


static int handle_parenthesis_not(char *line, enum token_type *data_type,
        char **data)
{
    if (strncmp(line, "(", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_LEFT_PARENTHESIS;
        return 1;
    }
    else if (strncmp(line, ")", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_RIGHT_PARENTHESIS;
        return 1;
    }
    else if (strncmp(line, "!", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_NOT;
        return 1;
    }
    return 0;
}


static int handle_bitwise(char *line, enum token_type *data_type,
        char **data)
{
    if (strncmp(line, "&", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_BITWISE_AND;
        return 1;
    }
    else if (strncmp(line, "|", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_BITWISE_OR;
        return 1;
    }
    else if (strncmp(line, "~", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_BITWISE_NOT;
        return 1;
    }
    else if (strncmp(line, "^", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_BITWISE_XOR;
        return 1;
    }
    return 0;
}


static int handle_basics_operators(char *line, enum token_type *data_type,
        char **data)
{
    if (strncmp(line, "+", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_PLUS;
        return 1;
    }
    else if (strncmp(line, "-", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_MINUS;
        return 1;
    }
    else if (strncmp(line, "*", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_MULTIPLY;
        return 1;
    }
    else if (strncmp(line, "/", 1) == 0)
    {
        *data = xstrndup(line, 1);
        *data_type = TOKEN_DIVIDE;
        return 1;
    }
    return 0;
}


static enum token_type get_type_and_set_data(char **line, char **data)
{
    // set default return type
    enum token_type data_type = TOKEN_PARAM;

    if (handle_and_or_power(*line, &data_type, data)) // && || **
        *line += 2;
    else if (handle_parenthesis_not(*line, &data_type, data)) // ( ) !
        *line += 1;
    else if (handle_bitwise(*line, &data_type, data)) // & | ^ ~
        *line += 1;
    else if (handle_basics_operators(*line, &data_type, data)) // + - * /
        *line += 1;
    else // token is a param
    {
        char *delim = strpbrk(*line, DELIMITERS);
        if (delim == NULL)
            delim = *line + strlen(*line); // put delim to the end of the line
        *data = xstrndup(*line, delim - *line);
        *line = delim;
    }

    return data_type;
}


static int get_priority(enum token_type type)
{
    if (type == TOKEN_PARAM)
        return -1;
    else if (type == TOKEN_AND || type == TOKEN_OR)
        return 0;
    else if (type == TOKEN_BITWISE_OR)
        return 1;
    else if (type == TOKEN_BITWISE_XOR)
        return 2;
    else if (type == TOKEN_BITWISE_AND)
        return 3;
    else if (type == TOKEN_PLUS || type == TOKEN_MINUS)
        return 4;
    else if (type == TOKEN_MULTIPLY || type == TOKEN_DIVIDE)
        return 5;
    else if (type == TOKEN_POWER)
        return 6;
    else if (type == TOKEN_NOT || type == TOKEN_BITWISE_NOT)
        return 7;

    //TODO: Minus & plus unary
    // parenthesis or error in given token
    return -2;
}


struct token *token_create(enum token_type type, int priority, char *data)
{
    struct token *new_token = xmalloc(sizeof(struct token));
    new_token->data = data;
    new_token->priority = priority;
    new_token->type = type;

    return new_token;
}

struct token *token_get_next(char **line)
{
    // skip blank and return NULL if there is no more tokens in line
    skip_blank(line);
    if (**line == '\0')
        return NULL;

    // get type and set data pointer to a new allocated string + move line
    char *new_data = NULL;
    enum token_type new_type = get_type_and_set_data(line, &new_data);

    skip_blank(line);

    // get priority of the token
    int new_priority = get_priority(new_type);

    // return token
    return token_create(new_type, new_priority, new_data);
}


void token_free(struct token **token_to_free)
{
    free((*token_to_free)->data);
    free(*token_to_free);
    *token_to_free = NULL;
}
