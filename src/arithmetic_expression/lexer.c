#include <stdlib.h>
#include <string.h>

#include "../memory/memory.h"
#include "lexer.h"

#define DELIMITERS " \t\n()+-*/|&^~!"

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
    char *delim = strpbrk(line, DELIMITERS);

    // data + move current position in line
    char *new_data = strndup(line, delim - line);

    // get type and priority of the token
    enum token_type new_type = get_type(new_data);
    int new_priority = get_priority(new_type);

    //return token
    return token_create(new_type, new_priority, new_data);
}

void token_free(struct token **token_to_free)
{
    free((*token_to_free)->data);
    free(*token_to_free);
    *token_to_free = NULL;
}