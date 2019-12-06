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

#define DELIMITERS " \\\n\t&|<>\"\'`$();#"
#define IFS " \n\t&|;()<>"

static void add_next_line_to_current_and_update_cursors(char **cursor,
        char **second_cursor);

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

char *get_delimiter(char *line, char *delimiters)
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

static int is_number(char *data)
{
    for (size_t i = 0; data[i]; i++)
    {
        if (data[i] < '0' || data[i] > '9')
            return 0;
    }

    return 1;
}

static int add_next_line_to_current_ending_with_backslash(char **cursor,
        char **second_cursor)
{
    char *next_line = get_next_line(g_env.prompt);
    if (next_line == NULL)
    {
        **cursor = '\0';
        return 0;
    }
    char *new_line = xcalloc(strlen(g_env.current_line)
                        + strlen(next_line) + 2, sizeof(char));

    // handle backslash as last char or add newline
    **cursor = '\0';
    strcat(new_line, g_env.current_line);

    // put cursor back where it was
    *cursor = *cursor - g_env.current_line + new_line;
    if (second_cursor != NULL)
        *second_cursor = *second_cursor - g_env.current_line + new_line;

    strcat(new_line, next_line);
    free(next_line);
    free(g_env.current_line);
    g_env.current_line = new_line;
    return 1;
}

static void handle_quoting(char **cursor, char **start_of_token)
{
    if (**cursor == '\'')
    {
        (*cursor)++;
        *cursor = get_delimiter(*cursor, "\'\0");
        while (**cursor != '\'') // end quote not found
        {
            add_next_line_to_current_and_update_cursors(cursor,
                    start_of_token);
            *cursor = get_delimiter(*cursor, "\'\0");
        }
        (*cursor)++;
    }
    else if (**cursor == '"')
    {
        (*cursor)++;
        *cursor = get_delimiter(*cursor, "\"\\\0");
        while (**cursor != '\"')
        {
            // Handle backslash
            if (**cursor == '\\' && *(*cursor + 1) != '\0')
                *cursor += 2;
            else if (**cursor == '\0' || **cursor == '\\')
            {
                add_next_line_to_current_and_update_cursors(cursor,
                        start_of_token);
            }
            *cursor = get_delimiter(*cursor, "\"\\\0");
        }
        (*cursor)++;
    }
    else if (**cursor == '\\')
    {
        if (*(*cursor + 1) == '\0') // if backslash at end of line
        {
            add_next_line_to_current_ending_with_backslash(cursor,
                    start_of_token);
        }
        else
        {
            *cursor += 2; // skip backslash and next char
        }
    }
}

static bool is_ifs(char current)
{
    for (char *c = IFS; *c != '\0'; c++)
    {
        if (current == *c)
            return true;
    }
    return current == '\0';
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
}

static void add_next_line_to_current_and_update_cursors(char **cursor,
        char **second_cursor)
{
    char *next_line = get_next_line(g_env.prompt);
    if (next_line == NULL)
        errx(2, "Lexing error");
    char *new_line = xcalloc(strlen(g_env.current_line)
                        + strlen(next_line) + 2, sizeof(char));

    // handle backslash as last char or add newline
    if (**cursor != '\\')
        strcat(strcat(new_line, g_env.current_line), "\n");
    else
    {
        **cursor = '\0';
        strcat(new_line, g_env.current_line);
    }

    // put cursor back where it was
    *cursor = *cursor - g_env.current_line + new_line;
    if (second_cursor != NULL)
        *second_cursor = *second_cursor - g_env.current_line + new_line;

    strcat(new_line, next_line);
    free(next_line);
    free(g_env.current_line);
    g_env.current_line = new_line;
}

void skip_quoting(char **cursor, char **start_of_token)
{
    if (**cursor == '\'')
    {
        (*cursor)++;
        *cursor = get_delimiter(*cursor, "\'");
        while (**cursor != '\'') // end quote not found
        {
            add_next_line_to_current_and_update_cursors(cursor,
                    start_of_token);
            *cursor = get_delimiter(*cursor, "\'");
        }
        (*cursor)++;
    }
    else if (**cursor == '"')
    {
        (*cursor)++;
        *cursor = get_delimiter(*cursor, "\"\\");
        while (**cursor != '\"')
        {
            // Handle backslash
            if (**cursor == '\\' && *(*cursor + 1) != '\0')
                *cursor += 2;
            else if (**cursor == '\0' || **cursor == '\\')
            {
                add_next_line_to_current_and_update_cursors(cursor,
                        start_of_token);
            }
            *cursor = get_delimiter(*cursor, "\"\\");
        }
        (*cursor)++;
    }
    else if (**cursor == '\\')
    {
        if (*(*cursor + 1) == '\0') // if backslash at end of line
        {
            add_next_line_to_current_ending_with_backslash(cursor,
                    start_of_token);
        }
        else
        {
            *cursor += 2; // skip backslash and next char
        }
    }
}

char *find_corresponding_parenthesis(char **cursor, char **token_start)
{
    int counter_bracket = 1;
    while (1)
    {
        while (**cursor != '\0' && counter_bracket != 0)
        {
            if (**cursor == '\'' || **cursor == '"' || **cursor == '\\')
            {
                skip_quoting(cursor, token_start);
                continue;
            }
            else if (**cursor == '(')
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
            add_next_line_to_current_and_update_cursors(cursor, token_start);
        }
    }
    return *cursor;
}

static bool is_delimiter(char current)
{
    for (char *c = DELIMITERS; *c != '\0'; c++)
    {
        if (current == *c)
            return true;
    }
    return current == '\0';
}

static void handle_dollar(char **cursor, char **token_start)
{
    // skip dollar
    (*cursor)++;

    if (**cursor == '(')
    {
        (*cursor)++;
        *cursor = find_corresponding_parenthesis(cursor, token_start);
    }
    else if (**cursor == '{')
    {
        (*cursor)++; //skip {
        if (**cursor == '}')
        {
            if (is_interactive())
                warnx("Bad substitution");
            else
                errx(1, "Bad substitution");
        }
        while (**cursor != '\0' && **cursor != '}')
            (*cursor)++;
        if (**cursor == '\0')
        {
            if (is_interactive())
                warnx("Bad lexing");
            else
                errx(2, "Bad lexing");
        }
        else
        {
            (*cursor)++;
        }
    }
    else //case $var
    {
        while (!is_delimiter(**cursor))
            (*cursor)++;
    }
}

static void handle_back_quote(char **cursor, char **token_start)
{
    (*cursor)++;

    *cursor = get_delimiter(*cursor, "`\0");
    while (**cursor != '`') // end back_quote not found
    {
        add_next_line_to_current_and_update_cursors(cursor, token_start);
        *cursor = get_delimiter(*cursor, "`\0");
    }
    (*cursor)++;
}

static enum token_lexer_type scan_token(char **cursor, char **token_start);

static void handle_var_assignement(char **cursor, char **token_start)
{
    (*cursor)++; //skip =
    scan_token(cursor, token_start);
}

static enum token_lexer_type scan_token(char **cursor, char **token_start)
{
    while (!is_ifs(**cursor))
    {
        if (**cursor == '\'' || **cursor == '"' || **cursor == '\\')
        {
            handle_quoting(cursor, token_start);
        }
        else if (**cursor == '$')
        {
            handle_dollar(cursor, token_start);
        }
        else if (**cursor == '`')
        {
            handle_back_quote(cursor, token_start);
        }
        else if (**cursor == '=')
        {
            handle_var_assignement(cursor, token_start);
            return TOKEN_ASSIGNEMENT;
        }
        else
            *cursor += 1;
    }
    return TOKEN_OTHER;
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
    static const char *reserved_words[] =
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

    char *token_start = cursor;
    enum token_lexer_type type = scan_token(&cursor, &token_start);
    free(new_token->data);
    set_token(new_token, type, &token_start, cursor - token_start);
    *delim = cursor;
    return new_token;
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

    if (*cursor == '#')
    {
        handle_comments(token_queue, delim, 0);
        free(new_token);
        return 0;
    }
    else if (cursor != *delim) // word pointed by cursor is not a delimiter
    {
        new_token = create_other_or_keyword_token(new_token, cursor, delim);
    }
    else
    {
        if (!generate_token_aux(token_queue, cursor, delim, new_token))
            return NULL;
    }

    return new_token;
}

struct queue *lexer(char *line, struct queue *token_queue)
{
    if (token_queue == NULL)
        token_queue = queue_init();

    char *cursor = line;
    handle_comments(token_queue, &cursor, 1);
    int is_first = 1;

    while (cursor != NULL && *cursor != '\0')
    {
        char *delim = get_delimiter(cursor, IFS);

        if (is_first)
        {
            char *alias = xstrndup(cursor, delim - cursor);
            char *alias_line;

            if ((alias_line = hash_find(g_env.aliases, alias)) != NULL)
            {
                lexer(alias_line, token_queue);
                cursor = delim;
                free(alias);
                is_first = 0;
                continue;
            }

            free(alias);
            free(alias_line);
        }

        struct token_lexer *token_found = generate_token(token_queue, cursor,
                &delim);
        if (token_found != NULL)
            queue_push(token_queue, token_found);
        cursor = delim;
        is_first = 0;
    }

    queue_push(token_queue, create_newline_token(NULL));
    return token_queue;
}

struct token_lexer *token_lexer_head(struct queue *token_queue)
{
    struct token_lexer *current_token = queue_head(token_queue);
    if (current_token != NULL)
        return current_token;

    // else
    if (g_env.is_parsing_ressource || !g_env.options.option_c)
        free(g_env.current_line);
    char *next_line = get_next_line(g_env.prompt);

    g_env.prompt = 2; //change prompt to ps2 with lexing

    g_env.current_line = next_line;

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
