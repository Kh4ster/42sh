#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "token_lexer.h"

#define DELIMITERS " \\\n\t&|<>$\"$();"

void skip_class(int (*classifier)(int c), char **cursor)
{
    while (classifier(**cursor))
        (*cursor)++;
}

void token_lexer_free(struct token_lexer **token)
{
    free((*token)->data);
    free(*token);
    *token = NULL;
}

char *get_delimiter(char *line)
{
    assert(line != NULL);
    char *delim = strpbrk(line, DELIMITERS);
    if (delim == NULL)
    { // find and return last char
        delim = line;
        while (*delim != '\0')
            delim++;
    }
    return delim;
}

char *xstrndup(char *str, size_t n)
{
    char *copy_str = strndup(str, n);
    if (copy_str == NULL)
        memory_exhausted();
    return copy_str;
}

void set_token(struct token_lexer *token, enum token_lexer_type token_type,
        char **cursor, size_t nb_char)
{
    token->data = xstrndup(*cursor, nb_char);
    token->type = token_type;
    *cursor += nb_char;
}

struct token_lexer *generate_token(char *cursor, char **delim)
{
    struct token_lexer *new_token = xmalloc(sizeof(struct token_lexer));

    if (cursor != *delim) // word pointed by cursor is not a delimiter
    {
        size_t token_length = *delim - cursor;
        new_token->data = xstrndup(cursor, token_length);
        new_token->type = TOKEN_OTHER;
    }
    /*
    else if (*cursor == '\"' || *cursor == '\\' || *cursor == '\'')
    {
        // handle_quoting(delim);
    }

    else if (*cursor == '$' || *cursor == '`')
    {
        // handle_sub_commands(delim);
    }
    */
    else if (strncmp(cursor, "&&", 2) == 0 || strncmp(cursor, "||", 2) == 0
            || strncmp(cursor, ";;", 2) == 0)
    {
        set_token(new_token, TOKEN_OPERATOR, delim, 2);
    }

    else if (*cursor == ';')
    {
        set_token(new_token, TOKEN_END_OF_INSTRUCTION, delim, 1);
    }

    else if (*cursor == '\n')
    {
        set_token(new_token, TOKEN_END_OF_LINE, delim, 1);
    }

    else if (*cursor == ' ' || *cursor == '\t')
    {
        skip_class(isblank, delim);
        free(new_token);
        return NULL;
    }

    else if (*cursor == '#')
    {
        while(**delim != '\n')
            (*delim)++;
    }

    else // other delimiters not defined yet
    {
        set_token(new_token, TOKEN_OTHER, delim, 1);
    }

    return new_token;
}

struct queue *lexer(char *line)
{
    struct queue *token_queue = queue_init();

    char *cursor = line;

    while (*cursor != '\0')
    {
        char *delim = get_delimiter(cursor);
        struct token_lexer *token_found = generate_token(cursor, &delim);
        if (token_found != NULL)
            queue_push(token_queue, token_found);
        cursor = delim;
    }

    return token_queue;
}