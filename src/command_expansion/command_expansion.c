#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../input_output/get_next_line.h"
#include "command_expansion.h"
#include "../execution_handling/command_container.h"
#include "../path_expention/path_exepension.h"
#include "../data_structures/data_string.h"
#include "../special_variables/expand_special_variables.h"
#include "../lexer/token_lexer.h"
#include "../memory/memory.h"
#include "../command_substitution/command_substitution.h"

#define IFS " \t\n"

extern int get_nb_params(char **params)
{
    int res = 0;

    for (int i = 0; params[i]; i++)
    {
        res++;
    }

    return res;
}

static char *expand_tilde(char **str)
{
    if (strcmp("~", *str) == 0)
        return strdup(getenv("HOME"));
    if (strcmp("~+", *str) == 0)
    {
        (*str)++;
        return get_current_dir_name();
    }
    if (strcmp("~-", *str) == 0)
    {
        (*str)++;
        return strdup(g_env.old_pwd);
    }
    return "mdr"; //never happens
}

static char* expand_path(char *str)
{
    struct path_globbing *glob = sh_glob(str);

    if (glob == NULL) //no match
        return NULL;

    struct string *string = string_init();
    for (int i = 0; i < glob->nb_matchs; ++i)
    {
        if (i != 0)
            string_append_char(string, ' ');
        string_append(string, glob->matches->content[i]);
    }

    destroy_path_glob(glob);
    return string_get_content(&string);
}

static void fill_command_and_params(struct command_container *command,
                                    char *expansion,
                                    struct array_list *parameters
)
{
    char *beg = expansion;
    free(command->command);
    command->command = strdup(strtok_r(expansion, IFS, &expansion));
    //first parameter is command
    array_list_append(parameters, strdup(command->command));

    char *param;
    while ((param = strtok_r(NULL, IFS, &expansion)) != NULL)
        array_list_append(parameters, strdup(param));
    free(beg);
}

static bool is_multiple_words(char *expansion)
{
    return strpbrk(expansion, IFS) != NULL;
}

static char *expand_variable(char **to_expand)
{
    char *special_variable = expand_special_variables(*to_expand);

    if (special_variable != NULL)
        return special_variable;

    if (**to_expand == '$' && *(*to_expand + 1) == '\0')
        return strdup("$");

    (*to_expand)++; //skip $
    char *value;

    char *beg = *to_expand;
    *to_expand = strpbrk(*to_expand, " $\'\"\\\n}{[]?!@`");
    if (*to_expand == NULL)
        *to_expand = beg + strlen(beg); //if null go to end

    char *var_name = strndup(beg, *to_expand - beg);

    if ((value = hash_find(g_env.variables, var_name)) == NULL)
        value = "";

    //case we jump to next char and ++ in scan will skip it so scan goes onto it
    (*to_expand)--;
    free(var_name);
    return strdup(value);
}

static char *expand_variable_brackets(char **to_expand)
{
    (*to_expand)++; //skip $
    (*to_expand)++; //skip {
    size_t i = 0;

    while (*(*to_expand + i) != '}')
        ++i;
    *(*to_expand + i) = '\0'; //remove }
    char *value;

    char *result = NULL;
    if ((value = hash_find(g_env.variables, *to_expand)) != NULL)
        result = strdup(value);
    else
        result = strdup("");

    *to_expand = (*to_expand + i);
    return result;
}

static bool is_to_expand(char c)
{
    return c == '$' || c == '\'' || c == '"' || c == '`' || c == '\\'
                                                                || c == '~';
}

char *scan_for_expand(char *line, bool is_quote, int *was_quote)
{
    //first check if can be a path expansion
    if (!is_quote && is_path_expansion(line))
    {
        char *result;
        if ((result = expand_path(line)) != NULL)
            return result;
    }

    struct string *new_line = string_init();

    for (; *line != '\0'; line++)
    {
        if (is_to_expand(*line))
        {
            char *expansion = expand(&line, is_quote, was_quote);
            string_append(new_line, expansion);
            free(expansion);
        }
        else
            string_append_char(new_line, *line);
    }
    return string_get_content(&new_line);
}

char *expand_quote(char **cursor, bool is_quote, int *was_quote)
{
    if (**cursor == '\'')
    {
        if (!is_quote) //is quote inside quote return quote
        {
            if (was_quote != NULL)
                *was_quote = 1;
            (*cursor)++;
            char *beg = *cursor;
            *cursor = get_delimiter(*cursor, "\'");
            return strndup(beg, *cursor - beg);
        }
        else
            return strdup("'");
    }
    else if (**cursor == '"')
    {
        (*cursor)++;
        if (was_quote != NULL)
            *was_quote = 2;
        char *beg = *cursor;
        *cursor = get_delimiter(*cursor, "\"\\");
        while (**cursor != '\"')
        {
            // Handle backslash
            if (**cursor == '\\')
                *cursor += 2;
            *cursor = get_delimiter(*cursor, "\"\\");
        }
        char *extracted_value = strndup(beg, *cursor - beg);
        char *result = scan_for_expand(extracted_value, true, was_quote);
        free(extracted_value);
        return result;
    }
    else if (**cursor == '\\' && !is_quote)// \ handling outside quotes
    {
        if (*(*cursor + 1) == '\0') /*line end with \*/
            return strdup("");
        *cursor = *cursor + 1;
        return strndup(*cursor, 1); // keep literal value after the backslash
    }
    else // handle backslash in quotes
    {
        (*cursor)++;
        if (**cursor == '$' || **cursor == '`' || **cursor == '"'
                || **cursor == '\\')
            return strndup(*cursor, 1);
        else
            return strndup(*cursor - 1, 2);
    }
}

static bool is_tidle(char *str)
{
    return strcmp(str, "~") == 0 || strcmp(str, "~+") == 0
                                                    || strcmp(str, "~-") == 0;
}

char *expand(char **to_expand, bool is_quote, int *was_quote)
{
    if (is_tidle(*to_expand))
        return expand_tilde(to_expand);
    if (**to_expand == '\'' || **to_expand == '"' || **to_expand == '\\')
        return expand_quote(to_expand, is_quote, was_quote);
    if (**to_expand == '$' && *(*to_expand + 1) == '(')
        return hat_of_expand_cmd(to_expand, ')', 2);
    if (**to_expand == '$' && *(*to_expand + 1) == '{')
        return expand_variable_brackets(to_expand);
    if (**to_expand == '$')
        return expand_variable(to_expand);
    if (**to_expand == '`')
        return hat_of_expand_cmd(to_expand, '`', 1);

    //strdup cause need to be able to free anything from expand
    return strdup(*to_expand); //no expansion
}

static void insert_sub_var(struct array_list *expanded_parameters,
                                            char *expansion,
                                            int *was_quote
)
{
    char *beg = expansion; //cause strtok_r will make expansion move
    if (*was_quote || !is_multiple_words(expansion))
    {
        array_list_append(expanded_parameters, expansion);
        return;
    }

    //first call without NULL
    array_list_append(expanded_parameters, strdup(strtok_r(expansion, IFS,
                                                                &expansion)));

    char *param;
    while ((param = strtok_r(NULL, IFS, &expansion)) != NULL)
        array_list_append(expanded_parameters, strdup(param));
    free(beg);
}

int handle_expand_command(struct instruction *command_i)
{
    struct command_container *command = command_i->data;
    struct array_list *expanded_parameters = array_list_init();

    char *expansion = scan_for_expand(command->command, false, NULL);
    if (*expansion == '\0' || *expansion == '\n') //expand empty var
    {
        free(expansion);
        size_t i = 1;
        while (command->params[i] != NULL) //while empty var we remove
        {
            expansion = scan_for_expand(command->params[i], false, NULL);
            if (*expansion == '\0' || *expansion == '\n')
            {
                free(expansion);
                i++;
            }
            else
                break;
        }
        if (command->params[i] == NULL) //only empty
        {
            array_list_destroy(expanded_parameters);
            return -1;
        }
    }

    if (is_multiple_words(expansion)) //var="echo ok ok ..."
        fill_command_and_params(command, expansion, expanded_parameters);
    else
    {
        free(command->command);
        command->command = expansion;
        array_list_append(expanded_parameters, strdup(command->command));
    }

    bool is_first = true; //to know first paramter ($a $b echo-> echo is first)
    for (size_t i = 0; command->params[i] != NULL; ++i)
    {
        int was_quote = 0;
        expansion = scan_for_expand(command->params[i], false, &was_quote);
        if (*expansion == '\0') //empty var
        {
            free(expansion);
            continue;
        }
        else
        {
            if (!is_first) //the first param being command already pushed
                insert_sub_var(expanded_parameters, expansion, &was_quote);
            else
                free(expansion);
            is_first = false;
        }
    }

    //create char ** to hold the new expanded parameters
    char **new_param_list = xmalloc((expanded_parameters->nb_element + 1)
                                                            * sizeof(char*));
    //fill it
    size_t i = 0;
    for (; i < expanded_parameters->nb_element; ++i)
        new_param_list[i] = expanded_parameters->content[i];
    new_param_list[i] = NULL;

    //free old list
    for (i = 0; command->params[i] != NULL; ++i)
        free(command->params[i]);
    free(command->params);
    free(expanded_parameters->content); //free array list but not content
    free(expanded_parameters);

    //set it
    command->params = new_param_list;

    return 1;
}