#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>

#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "token_lexer.h"
#include "../input_output/get_next_line.h"
#include "../execution_handling/command_container.h"
#include "../execution_handling/command_execution.h"
#include "../error/error.h"

#define DELIMITERS " \\\n\t&|<>$\"\'`$();"

static void skip_class(int (*classifier)(int c), char **cursor)
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

static char *get_delimiter(char *line, char *delimiters)
{
    assert(line != NULL);
    char *delim = strpbrk(line, delimiters);
    if (delim == NULL)
    { // find and return last char
        delim = line;
        while (*delim != '\0')
            delim++;
    }
    return delim;
}

static char *xstrndup(char *str, size_t n)
{
    char *copy_str = strndup(str, n);
    if (copy_str == NULL)
        memory_exhausted();
    return copy_str;
}

static struct token_lexer *create_newline_token(struct token_lexer *new_token)
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

static int is_number(char *data)
{
    for (size_t i = 0; data[i]; i++)
    {
        if (data[i] < '0' || data[i] > '9')
            return 0;
    }

    return 1;
}


static struct token_lexer *create_other_or_keyword_token(
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
    for (; i < sizeof(reserved_words) / sizeof(char *); i++)
    {
        if (strcmp(reserved_words[i], new_token->data) == 0)
        {
            new_token->type = TOKEN_KEYWORD;
            return new_token;
        }
    }

    if (is_assignement(new_token->data))
        new_token->type = TOKEN_ASSIGNEMENT;

    else
        new_token->type = TOKEN_OTHER;

    return new_token;
}


static void set_token(struct token_lexer *token,
                enum token_lexer_type token_type,
                char **cursor,
                size_t nb_char
)
{
    token->data = xstrndup(*cursor, nb_char);
    token->type = token_type;
    *cursor += nb_char;
}

static void handle_comments(struct queue *token_queue,
                    struct token_lexer *new_token,
                    char **cursor,
                    int is_linestart)
{
    if (*cursor == NULL)
        return;
    if (**cursor != '#')
        return;

    if (is_linestart || *(*cursor - 1) == ' ')
    {
        while (**cursor != '\0' && **cursor != '\n')
            (*cursor)++;
    }
    else
    {
        char *delim = get_delimiter(*cursor + 1, DELIMITERS);
        struct token_lexer *last_token = token_queue->tail->data;
        last_token->data = realloc(last_token->data, delim - *cursor
                                   + strlen(last_token->data) + 1);
        last_token->data = strncat(last_token->data, *cursor, delim - *cursor);
        *cursor = delim;
    }
    free(new_token);
}

char *find_corresponding_bracket(char **cursor, char **token_start)
{
    int counter_bracket = 1;
    while (1)
    {
        while (**cursor != '\0' && counter_bracket != 0)
        {
            //TODO skip_quoting()
            if (**cursor == '(')
                counter_bracket++;
            else if (**cursor == ')')
                counter_bracket--;
            (*cursor)++;
        }
        if (counter_bracket == 0)
        {
            break;
        }
        if (**cursor == '\0')
        {
            char *next_line = get_next_line(g_env.prompt);
            if (next_line == NULL)
                errx(2, "Lexing error");

            char *new_line = xcalloc(strlen(*token_start)
                                + strlen(next_line) + 1, sizeof(char));
            strcat(new_line, *token_start);
            strcat(new_line, next_line);
            free(next_line);
            *cursor = new_line + (*cursor - *token_start);
            *token_start = new_line;
        }
    }
    return *cursor;
}

static void handle_dollar(struct token_lexer *new_token, char **cursor)
{
    // keep beginning of the dollar expression
    char *token_start = *cursor;

    // skip dollar
    (*cursor)++;

    if (**cursor == '\0')
        set_token(new_token, TOKEN_OTHER, cursor, 1);
    else if (**cursor == '(')
    {
        (*cursor)++;
        char *end_bracket = find_corresponding_bracket(cursor, &token_start);
        set_token(new_token, TOKEN_OTHER, &token_start,
                end_bracket - token_start);
    }
    else //case $var
    {
        while (**cursor && **cursor != ' ')
            (*cursor)++;
        set_token(new_token, TOKEN_OTHER, &token_start,
                *cursor - token_start);
    }
}

static void handle_quoting(struct token_lexer *new_token,
        char **cursor)
{
    if (**cursor == '\'')
    {
        (*cursor)++;
        char *end_quote = strchr(*cursor, '\'');
        #if 0
        while (end_quote == NULL)
        {
            // TODO strcat get_next_line to the current str
            // end_quote = strchr(*cursor, '\'');
            // if EOF, error, expecting a closing '
        }
        #endif /* 0 */
        set_token(new_token, TOKEN_OTHER, cursor, end_quote - *cursor);
        *cursor = end_quote + 1;
    }
    else if (**cursor == '"')
    {
        (*cursor)++;
        char *end_quote = get_delimiter(*cursor, "\"\\\0");
        while (*end_quote != '\"')
        {
            // Handle backslash
            if (*end_quote == '\\' && *(end_quote + 1) != '\0')
                end_quote += 2;
            if (*end_quote == '\0')
                break; // TODO strcat get_next_line to the current str
            // if EOF : error, expecting a closing "
            end_quote = get_delimiter(*cursor, "\"\\\0");
        }
        set_token(new_token, TOKEN_OTHER, cursor, end_quote - *cursor);
        *cursor = end_quote + 1;
    }
}

static void handle_io_number(char *cursor, struct queue *token_queue)
{
    if (token_queue->size && (! strncmp(cursor, ">>", 2)
            || ! strncmp(cursor, ">", 1) || ! strncmp(cursor, "<", 1)
            || ! strncmp(cursor, ">&", 2) || ! strncmp(cursor, "<>", 2)
            || !strncmp(cursor, "<&", 2) || ! strncmp(cursor, "<<", 2)
            || !strncmp(cursor, "<<-", 3)))
    {
        char *is_delim = cursor - 1;
        if (strpbrk(is_delim, DELIMITERS) != is_delim)
        {
            if (token_queue->tail)
            {
                struct token_lexer *token = token_queue->tail->data;
                if (is_number(token->data))
                    token->type = TOKEN_IO_NUMBER;
            }
        }
    }
}

static int generate_token_redirection(struct queue *token_queue, char *cursor,
        char **delim, struct token_lexer *new_token)
{
    if (strncmp(cursor, "<<-", 3) == 0)
    {
        handle_io_number(cursor, token_queue);
        set_token(new_token, TOKEN_OPERATOR, delim, 3);
        return 1;
    }


    if (strncmp(cursor, "&&", 2) == 0 || strncmp(cursor, "||", 2) == 0
            || strncmp(cursor, ";;", 2) == 0 || strncmp(cursor, ">>", 2) == 0
            || strncmp(cursor, ">&", 2) == 0 || strncmp(cursor, "<>", 2) == 0
        || strncmp(cursor, "<&", 2) == 0 || strncmp(cursor, ">|", 2) == 0
        || strncmp(cursor, "<<", 2) == 0)
    {
        handle_io_number(cursor, token_queue);
        set_token(new_token, TOKEN_OPERATOR, delim, 2);
        return 1;
    }

    if (strncmp(cursor, "|", 1) == 0)
    {
        set_token(new_token, TOKEN_OPERATOR, delim, 1);
        return 1;
    }

    else if (! strncmp(cursor, ">", 1) || ! strncmp(cursor, "<", 1))
    {
        handle_io_number(cursor, token_queue);
        set_token(new_token, TOKEN_OPERATOR, delim, 1);
        return 1;
    }
    return 0;
}

static int generate_token_aux(struct queue *token_queue, char *cursor,
        char **delim, struct token_lexer *new_token)
{
    if (generate_token_redirection(token_queue, cursor, delim, new_token))
        return 1;

    if (strncmp(cursor, "\\n", 2) == 0)
    {
        new_token = create_newline_token(new_token);
        (*delim) += 2;
    }

    else if (*cursor == ';' || *cursor == '&')
    {
        set_token(new_token, TOKEN_END_OF_INSTRUCTION, delim, 1);
    }

    else if (*cursor == ' ' || *cursor == '\t')
    {
        skip_class(isblank, delim);
        free(new_token);
        return 0;
    }

    else if (*cursor == '#')
    {
        handle_comments(token_queue, new_token, delim, 0);
        return 0;
    }

    else // other delimiters not defined yet
    {
        set_token(new_token, TOKEN_OTHER, delim, 1);
    }
    return 1;
}

static struct token_lexer *generate_token(struct queue *token_queue,
                                    char *cursor,
                                    char **delim
)
{
    if (*cursor == '\0')
        return NULL;

    struct token_lexer *new_token = xmalloc(sizeof(struct token_lexer));

    if (cursor != *delim) // word pointed by cursor is not a delimiter
    {
        new_token = create_other_or_keyword_token(new_token, cursor, delim);
    }

    else if (*cursor == '\"' || *cursor == '\'')
    {
        handle_quoting(new_token, delim);
    }

    #if 0
    else if (*cursor == '\\')
    {
        handle_escape(delim);
    }
    #endif /* 0 */
    else if (*cursor == '$' || *cursor == '`')
    {
        handle_dollar(new_token, delim);
    }

    else
        if (!generate_token_aux(token_queue, cursor, delim, new_token))
            return NULL;

    return new_token;
}

struct queue *lexer(char *line, struct queue *token_queue)
{
    if (token_queue == NULL)
        token_queue = queue_init();

    char *cursor = line;
    handle_comments(token_queue, NULL, &cursor, 1);
    while (cursor != NULL && *cursor != '\0')
    {
        char *delim = get_delimiter(cursor, DELIMITERS);
        struct token_lexer *token_found = generate_token(token_queue, cursor,
                &delim);
        if (token_found != NULL)
            queue_push(token_queue, token_found);
        cursor = delim;
    }

    queue_push(token_queue, create_newline_token(NULL));
    return token_queue;
}

struct token_lexer *token_lexer_head(struct queue *token_queue)
{
    struct token_lexer *current_token = queue_head(token_queue);
    if (current_token != NULL)
        return current_token;

    free(g_env.current_line);
    char *next_line = get_next_line(g_env.prompt);

    g_env.prompt = "> "; //change prompt to ps2 with lexing

    if (next_line == NULL) // End Of File
    {
        g_env.current_line = NULL;
        current_token = xmalloc(sizeof(struct token_lexer));
        current_token->type = TOKEN_EOF;
        current_token->data = strdup("ouais");
        queue_push(token_queue, current_token);
    }
    else
    {
        token_queue = lexer(next_line, token_queue);
        current_token = token_lexer_head(token_queue);

        /*
        ** free line if not -c option, execpt if its parsing ressource
        ** then even if 42sh binary was called with -c we want to free lines
        */
        if (g_env.is_parsing_ressource || !g_env.options.option_c)
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
    while ((*token_queue)->size != 0)
    {
        struct token_lexer *to_free = token_lexer_pop(*token_queue);
        token_lexer_free(&to_free);
    }
    queue_destroy(token_queue);
}

void token_queue_empty(struct queue *token_queue)
{
    while (token_queue->size != 0)
    {
        struct token_lexer *to_free = token_lexer_pop(token_queue);
        token_lexer_free(&to_free);
    }
}

struct token_lexer *lexer_next_next(struct queue *token_queue)
{
    struct token_lexer *head = queue_pop(token_queue);
    struct token_lexer *next_next = queue_head(token_queue);
    queue_push_start(token_queue, head);
    return next_next;
}
