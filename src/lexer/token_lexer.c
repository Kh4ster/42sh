#include <string.h>
#include <assert.h>

#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "token_lexer.h"

#define DELIMITERS " \\\n\t&|<>$\"$()"

void skip_class(int (*classifier)(int c), char **cursor)
{
    while (classifier(**cursor))
        (*cursor)++;
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

struct token_lexer *generate_token(char *cursor, char **delim)
{
    struct token_lexer *new_token = xmalloc(sizeof(struct token_lexer));

    if (cursor != *delim) // word pointed by cursor is not a delimiter
    {
        size_t token_length = *delim - cursor;
        new_token->data = xstrndup(cursor, token_length);
        new_token->type = TOKEN_OTHER;
        return new_token;
    }

    if (*cursor == '\n')
    {
        new_token->data = xstrndup(cursor, 1);
        new_token->type = TOKEN_END_OF_LINE;
        delim += 1;
    }
    else if (*cursor == ';')
    {
        new_token->data = xstrndup(cursor, 1);
        new_token->type = TOKEN_END_OF_INSTRUCTION;
        delim += 1;
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
        if (delim == cursor)
            break;
        struct token_lexer *token_found = generate_token(cursor, &delim);
        if (token_found != NULL)
            queue_push(token_queue, token_found);
        cursor = delim;
    }

    return token_queue;
}