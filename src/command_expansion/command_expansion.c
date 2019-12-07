#include <signal.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>
#include <fnmatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>

#include "../parser/ast/ast.h"
#include "../parser/parser.h"
#include "../lexer/token_lexer.h"
#include "../execution_handling/command_container.h"
#include "../execution_handling/command_execution.h"
#include "../redirections_handling/redirect.h"
#include "../data_structures/hash_map.h"
#include "../input_output/get_next_line.h"
#include "../builtins/break.h"
#include "../path_expention/path_exepension.h"
#include "../error/error.h"
#include "../memory/memory.h"
#include "../data_structures/array_list.h"
#include "../data_structures/data_string.h"
#include "../special_variables/expand_special_variables.h"
#include "../path_expention/path_exepension.h"
#include "../arithmetic_expression/parser.h"
#include "../arithmetic_expression/tree.h"
#include "../command_substitution/command_substitution.h"
#include "../command_expansion/command_expansion.h"
#include "command_expansion.h"

// IFS " \t\n"
#define MARK_IFS -5

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
        if (g_env.old_pwd)
        {
            (*str)++;
            return strdup(g_env.old_pwd);
        }
    }

    char to_return[3] = "~";
    if (*(*str + 1) == '+' || *(*str + 1) == '-')
    {
        (*str)++;
        to_return[1] = **str;
        to_return[2] = '\0';
    }

    return strdup(to_return); //never happens
}


static char *expand_path(char *str)
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
    command->command = strdup(strtok_r(expansion,
                                hash_find(g_env.variables, "IFS"), &expansion));
    //first parameter is command
    array_list_append(parameters, strdup(command->command));

    char *param;
    while ((param = strtok_r(NULL,
                        hash_find(g_env.variables, "IFS"), &expansion)) != NULL)
        array_list_append(parameters, strdup(param));
    free(beg);
}


static bool is_multiple_words(char *expansion)
{
    return strpbrk(expansion, " \n\t") != NULL;
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
    while ((**to_expand >= 'A' && **to_expand <= 'Z')
            || **to_expand == '_'
            || (**to_expand >= 'a' && **to_expand <= 'z')
            || (**to_expand >= '0' && **to_expand <= '9'))
        (*to_expand)++;

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

static bool contains_space(char *ifs)
{
    while (*ifs != '\0')
    {
        if (*ifs == ' ')
            return true;
        ifs++;
    }
    return false;
}

static void replace_ifs_with_mark(char *expansion)
{
    char *ifs = hash_find(g_env.variables, "IFS");
    for (size_t i = 0; expansion[i] != '\0'; ++i)
    {
        for (size_t j = 0; ifs[j] != '\0'; ++j)
        {
            //overide for the space case cause differ for default content
            if (contains_space(ifs) && expansion[i] == ' ')
            {
                expansion[i] = ' ';
                break;
            }
            if (expansion[i] == ifs[j])
            {
                if (expansion[i + 1] == 0) //last char
                    expansion[i] = '\0';
                else
                    expansion[i] = MARK_IFS;
                break;
            }
        }
    }
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
            replace_ifs_with_mark(expansion);
            string_append(new_line, expansion);
            free(expansion);
        }
        else
            string_append_char(new_line, *line);
    }

    return string_get_content(&new_line);
}

static char *replace_mark_with_space(char *expansion)
{
    for (size_t i = 0; expansion[i] != '\0'; ++i)
    {
        if (expansion[i] == MARK_IFS)
            expansion[i] = ' ';
    }
    return expansion;
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
    return *str == '~';
}

static char *handle_expand_arithmetic(char **to_expand)
{
    char *begin = *to_expand + 3;

    char *end = strstr(begin, "))");
    char *to_compute = strndup(begin, end - begin);
    size_t jump = strlen(to_compute);

    char *new_to_compute = scan_for_expand(to_compute, true, NULL);
    free(to_compute);
    to_compute = new_to_compute;
    struct node *root = parser(to_compute);
    int result = 1;

    if (root)
    {
        result = evaluate_tree(root);
        destroy_ar_tree(root);
    }

    char *to_return = NULL;

    int error = asprintf(&to_return, "%d", result);

    *to_expand += jump + 4;

    free(to_compute);

    if (error == -1)
        return NULL;
    return to_return;
}


static char *expand_if_special_variable(char **to_expand)
{
    char *result;
    char *end_spec;
    char *cpy_expansion = strdup(*to_expand);

    if (*cpy_expansion + 1 == '{')
        end_spec = strpbrk(cpy_expansion, "}");
    else
        end_spec = strpbrk(cpy_expansion + 1, "0123456789#$@?*");

    if (end_spec)
    {
        *(end_spec + 1) = '\0';
    }

    result = expand_special_variables(cpy_expansion);

    if (result != NULL)
        *to_expand += strlen(cpy_expansion) - 1;

    free(cpy_expansion);
    return result;
}

char *expand(char **to_expand, bool is_quote, int *was_quote)
{
    if (is_tidle(*to_expand))
        return expand_tilde(to_expand);
    if (**to_expand == '\'' || **to_expand == '"' || **to_expand == '\\')
        return expand_quote(to_expand, is_quote, was_quote);
    if (strncmp(*to_expand, "$((", 3) == 0)
        return handle_expand_arithmetic(to_expand);
    if (**to_expand == '$' && *(*to_expand + 1) == '(')
        return hat_of_expand_cmd(to_expand, ')', 2);

    if (**to_expand == '$')
    {
        char *result;

        if ((result = expand_if_special_variable(to_expand)) != NULL)
            return result;
    }

    if (**to_expand == '$' && *(*to_expand + 1) == '{')
        return expand_variable_brackets(to_expand);
    if (**to_expand == '$')
        return expand_variable(to_expand);
    if (**to_expand == '`')
        return hat_of_expand_cmd(to_expand, '`', 1);
    if (is_path_expansion(*to_expand) && !is_quote && !*was_quote)
        return expand_path(*to_expand);

    //strdup cause need to be able to free anything from expand
    char *to_return = strdup(*to_expand); //no expansion
    *to_expand += strlen(*to_expand) - 1;
    return to_return;
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
    array_list_append(expanded_parameters, strdup(strtok_r(expansion,
                                            " \n\t", &expansion)));

    char *param;
    while ((param = strtok_r(NULL, " \n\t",&expansion)) != NULL)
                        array_list_append(expanded_parameters, strdup(param));
    free(beg);
}

static bool is_empty_var(char *expansion)
{
    char *save = strdup(expansion);
    char *beg = save;
    bool result = strtok_r(save,
                    hash_find(g_env.variables, "IFS"), &save) == NULL;
    free(beg);
    return result;
}

int handle_expand_command(struct instruction *command_i)
{
    struct command_container *command = command_i->data;
    struct array_list *expanded_parameters = array_list_init();

    char *expansion = scan_for_expand(command->command, false, NULL);
    if (is_empty_var(expansion)) //expand empty var
    {
        free(expansion);
        size_t i = 1;
        while (command->params[i] != NULL) //while empty var we remove
        {
            expansion = scan_for_expand(command->params[i], false, NULL);
            if (is_empty_var(expansion))
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
        if (is_empty_var(expansion)) //empty var
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
        new_param_list[i] = replace_mark_with_space(
                                            expanded_parameters->content[i]);
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