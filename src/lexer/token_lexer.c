#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "token_lexer.h"
#include "../input_output/get_next_line.h"

#define DELIMITERS " \\\n\t&|<>$\"\'`$();"

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

    else if (strncmp(cursor, "\\n", 2) == 0)
    {
        char *newline_str = xcalloc(2, sizeof(char));
        newline_str[0] = '\n';
        new_token->data = newline_str;
        new_token->type = TOKEN_OTHER;
        (*delim) += 2;
    }

    else if (*cursor == ';')
    {
        set_token(new_token, TOKEN_END_OF_INSTRUCTION, delim, 1);
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

struct queue *lexer(char *line, struct queue *token_queue)
{
    if (token_queue == NULL)
        token_queue = queue_init();

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

struct token_lexer *token_lexer_head(struct queue *token_queue)
{
    struct token_lexer *current_token = queue_head(token_queue);
    if (current_token != NULL)
        return current_token;
    char *next_line = get_next_line(g_env.prompt);
    // TODO add_history(next_line);
    g_env.prompt = ">";
    if (next_line == NULL) // End Of File
    {
        current_token = xmalloc(sizeof(struct token_lexer));
        current_token->type = TOKEN_EOF;
        current_token->data = strdup("ouais");
        queue_push(token_queue, current_token);
    }
    else
    {
        token_queue = lexer(next_line, token_queue);
        current_token = token_lexer_head(token_queue);
        if (g_env.options.option_c != 1)
            free(next_line);
    }
    return current_token;
}

struct token_lexer *token_lexer_pop(struct queue *token_queue)
{
    struct token_lexer *current_token = token_lexer_head(token_queue);
    queue_pop(token_queue);
    return current_token;
}

void token_queue_free(struct queue **token_queue)
{
    while((*token_queue)->size != 0)
    {
        struct token_lexer *to_free = token_lexer_pop(*token_queue);
        token_lexer_free(&to_free);
    }
    queue_destroy(token_queue);
}
