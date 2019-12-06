#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../memory/memory.h"
#include "../data_structures/hash_map.h"
#include "../execution_handling/command_execution.h"
#include "../command_expansion/command_expansion.h"
#include "../data_structures/data_string.h"
#include "command_substitution.h"
#include "../lexer/token_lexer.h"

//to know where the variable start
#define DELIMITERS " \\\n\t&|<>\"\'`$();#"

//to really understand take this example $(echo $(echo ok))
static char *expand_nested_command(char *cursor,
                                    char *to_expand,
                                    char char_stopped_on,
                                    struct hash_map *inner_var
)
{
    char *result;
    if (char_stopped_on == '$')
        result = expand_cmd(cursor, ')', 2, inner_var, false);
    else
        result = expand_cmd(cursor, '`', 1, inner_var, false);
    char *end = cursor;
    end += strlen(end); //move to \0 set in recursive call (matching ))
    end++; //skip \0

    *cursor = 0; //set $ to \0

    char *new_to_expand = xcalloc(strlen(to_expand)
                                    + strlen(result)
                                    + strlen(end) + 1, sizeof(char));

    strcat(new_to_expand, to_expand);
    strcat(new_to_expand, result);
    strcat(new_to_expand, end);
    free(result);
    return new_to_expand;
}

static bool is_not_delimiter(char caracter)
{
    for (char *c = DELIMITERS; *c != '\0'; c++)
    {
        if (caracter == *c)
            return false;
    }
    return true;
}

static void add_to_inner_var(char *beg, char *cursor, struct hash_map *inner_var)
{
    char *equal_sign = cursor;
    char *var_value = equal_sign + 1;
    while (*var_value != '\0' && is_not_delimiter(*var_value))
        var_value++;
    while (cursor >= beg && is_not_delimiter(*cursor))
        cursor--;
    cursor++; //skip symbol
    char *var_name = strndup(cursor, equal_sign - cursor);
    var_value = strndup(equal_sign + 1, var_value - equal_sign);
    hash_insert(inner_var, var_name, var_value, STRING);
    free(var_name);
    free(var_value);
}

static bool is_not_inner_var(char *line, struct hash_map *inner_var)
{
    if (*line != '$') //if not dollar, it's not inner var
        return true;
    line++; //skip dollar
    if (*(line + 1) == '{')
        line++;

    char *delim = strpbrk(line, "$\'\"\\\n}{[]?!@`");
    if (delim == NULL) //did not match so go to end of line
         delim = line + strlen(line);

    char *var_name = strndup(line, delim - line);
    void *result = hash_find(inner_var, var_name);
    free(var_name);
    return result == NULL;
}

static bool custom_is_to_expand(char c)
{
    return c == '$' || c == '"';
}

static char* expand_inner_var(char **to_expand, struct hash_map *inner_var)
{
    if (**to_expand == '$' && *(*to_expand + 1) == '\0')
        return strdup("$");

    (*to_expand)++; //skip $
    char *value;

    char *beg = *to_expand;
    *to_expand = strpbrk(*to_expand, " $\'\"\\\n}{[]?!@`");
    if (*to_expand == NULL)
        *to_expand = beg + strlen(beg); //if null go to end

    char *var_name = strndup(beg, *to_expand - beg);

    if ((value = hash_find(inner_var, var_name)) == NULL)
        value = "";

    //case we jump to next char and ++ in scan will skip it so scan goes onto it
    (*to_expand)--;
    free(var_name);
    return strdup(value);
}

char *custom_scan(char *line,
                        bool is_quote,
                        int *was_quote,
                        struct hash_map *inner_var
)
{
    struct string *new_line = string_init();

    for (; *line != '\0'; line++)
    {
        if (custom_is_to_expand(*line) && is_not_inner_var(line, inner_var))
        {
            if (*line == '"')
            {
                *was_quote = 2;
                if (is_not_inner_var(line + 1, inner_var) && *(line + 1) == '$')
                {
                    char *expansion = expand(&line, is_quote, was_quote);
                    string_append(new_line, expansion);
                    free(expansion);
                }
                else
                {
                    string_append_char(new_line, *line);
                    line++;
                    while (*line != '"')
                    {
                        string_append_char(new_line, *line);
                        line++;
                    }
                    string_append_char(new_line, *line);
                }
            }
            else
            {
                char *expansion = expand(&line, is_quote, was_quote);
                string_append(new_line, expansion);
                free(expansion);
            }
        }
        else if (!is_not_inner_var(line, inner_var))
        {
            char *expansion = expand_inner_var(&line, inner_var);
            string_append(new_line, expansion);
            free(expansion);
        }
        else
        {
            if (*line == '\'')
            {
                *was_quote = 1;
                string_append_char(new_line, *line);
                line++;
                while (*line != '\'')
                {
                    string_append_char(new_line, *line);
                    line++;
                }
            }
            string_append_char(new_line, *line);
        }
    }
    return string_get_content(&new_line);
}

char *expand_cmd(char *to_expand,
                        char to_stop,
                        int nb_to_skip,
                        struct hash_map *inner_var,
                        bool is_first
)
{
    bool to_free = false;
    to_expand += nb_to_skip;

    char *cursor = to_expand;
    while ((cursor = strpbrk(cursor, "=`$)\"\'\\")) != NULL
                                                        && *cursor != to_stop)
    {
        if (*cursor == '\"' || *cursor == '\'' || *cursor == '\\')
            skip_quoting(&cursor, NULL);
        else
        {
            //recursive call to expand command
            if ((*cursor == '$' && cursor[1] == '(') || *cursor == '`')
            {
                char *expansion = expand_nested_command(cursor,
                                                    to_expand,
                                                    *cursor,
                                                    inner_var);
                if (to_free) //already expanded a first one
                    free(to_expand);
                to_expand = expansion;
                cursor = to_expand;
                to_free = true;
            }
            else if (*cursor == '$')    //if we fall on a $ for a var we need to skip it
                cursor++;
            else if (*cursor == '=')    //fell on a var assignement
            {
                add_to_inner_var(to_expand, cursor, inner_var);
                cursor++;
            }
        }
    }

    *cursor = '\0'; //replace ) with 0

    char *result = get_result_from_42sh(to_expand, inner_var, is_first);
    if (to_free) //inner expansion that needs to be freed
        free(to_expand);
    return result;
}

char *hat_of_expand_cmd(char **to_expand,
                                        char to_stop,
                                        int nb_to_skip
)
{
    struct hash_map inner_var;
    hash_init(&inner_var, NB_SLOTS);

    char *beg = strdup(*to_expand);
    char *end_parenthesis = beg + nb_to_skip;
    find_corresponding_parenthesis(&end_parenthesis, NULL);
    *to_expand += (end_parenthesis - beg - 1);
    *end_parenthesis = '\0';
    char *result = expand_cmd(beg, to_stop, nb_to_skip, &inner_var, true);
    hash_free(&inner_var);
    free(beg);
    return result;
}