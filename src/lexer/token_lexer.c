#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "token_lexer.h"
#include "../input_output/get_next_line.h"

#define DELIMITERS " \\\n\t&|<>$\"\'`$();2"

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

struct token_lexer *create_newline_token(struct token_lexer *new_token)
{
    if (new_token == NULL)
        new_token = xmalloc(sizeof(struct token_lexer));
    new_token->data = strdup("\n");
    new_token->type = TOKEN_OTHER;
    return new_token;
}

static int is_assignement(char *data)
{
    while (*data)
    {
        if (*data == '=')
            return 1;
        data++;
    }
    return 0;
}

struct token_lexer *create_other_or_keyword_token(
       struct token_lexer *new_token,
       char *cursor,
       char **delim
)
{
    size_t token_length = *delim - cursor;
    new_token->data = xstrndup(cursor, token_length);

    // is word a keyword or something else
    const char *reserved_words[] =
    {
        "!", "{", "}", "case", "do", "done", "elif", "else",
        "esac", "fi", "for", "if", "in", "then", "until", "while"
    };
    size_t i = 0;
    for (; i < sizeof(reserved_words) / sizeof(char*); i++)
    {
        if (strcmp(reserved_words[i], new_token->data) == 0)
        {
            new_token->type = TOKEN_KEYWORD;
            break;
        }
    }
    if (i == sizeof(reserved_words) / sizeof(char*))
    {
        if (is_assignement(new_token->data))
            new_token->type = TOKEN_ASSIGNEMENT;
        else
            new_token->type = TOKEN_OTHER;
    }
    return new_token;
}

void set_token(struct token_lexer *token,
                enum token_lexer_type token_type,
                char **cursor,
                size_t nb_char
)
{
    token->data = xstrndup(*cursor, nb_char);
    token->type = token_type;
    *cursor += nb_char;
}

void handle_comments(struct queue *token_queue,
                    struct token_lexer *new_token,
                    char **cursor,
                    int is_linestart
)
{
    if (**cursor != '#')
        return;
    if (is_linestart || *(*cursor - 1) == ' ')
    {
        while (**cursor != '\0' && **cursor != '\n')
            (*cursor)++;
    }
    else
    {
        char *delim = get_delimiter(*cursor + 1);
        struct token_lexer *last_token = token_queue->tail->data;
        last_token->data = realloc(last_token->data, delim - *cursor
                                   + strlen(last_token->data) + 1);
        last_token->data = strncat(last_token->data, *cursor, delim - *cursor);
        *cursor = delim;
    }
    free(new_token);
}

struct token_lexer *generate_token(struct queue *token_queue,
                                    char *cursor,
                                    char **delim
)
{
    struct token_lexer *new_token = xmalloc(sizeof(struct token_lexer));

    if (cursor != *delim) // word pointed by cursor is not a delimiter
    {
        new_token = create_other_or_keyword_token(new_token, cursor, delim);
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
            || strncmp(cursor, ";;", 2) == 0 || strncmp(cursor, ">>", 2) == 0
            || strncmp(cursor, "2>", 2) == 0)
    {
        set_token(new_token, TOKEN_OPERATOR, delim, 2);
    }

    else if (strncmp(cursor, "\\n", 2) == 0)
    {
        new_token = create_newline_token(new_token);
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
        handle_comments(token_queue, new_token, delim, 0);
        return NULL;
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
    handle_comments(token_queue, NULL, &cursor, 1);
    while (*cursor != '\0')
    {
        char *delim = get_delimiter(cursor);
        struct token_lexer *token_found = generate_token(token_queue, cursor
                                                         , &delim);
        if (token_found != NULL)
            queue_push(token_queue, token_found);
        cursor = delim;
    }
    if (is_interactive())
        queue_push(token_queue, create_newline_token(NULL));
    return token_queue;
}

struct token_lexer *token_lexer_head(struct queue *token_queue)
{
    struct token_lexer *current_token = queue_head(token_queue);
    if (current_token != NULL)
        return current_token;
    char *next_line = get_next_line(g_env.prompt);
    // TODO add_history(next_line);
    if (next_line == NULL) // End Of File
    {
        current_token = xmalloc(sizeof(struct token_lexer));
        current_token->type = TOKEN_EOF;
        current_token->data = strdup("ouais");
        queue_push(token_queue, current_token);
    }
    else
    {
        if (g_env.not_first_line)
            queue_push(token_queue, create_newline_token(NULL));
        token_queue = lexer(next_line, token_queue);
        current_token = token_lexer_head(token_queue);
        if (g_env.options.option_c != 1)
            free(next_line);
    }
    g_env.not_first_line = 1;
    g_env.prompt = "> ";
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
