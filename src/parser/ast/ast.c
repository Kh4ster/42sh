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

#include "ast.h"
#include "../parser.h"
#include "../../lexer/token_lexer.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"
#include "../../redirections_handling/redirect.h"
#include "../../data_structures/hash_map.h"
#include "../../input_output/get_next_line.h"
#include "../../builtins/break.h"
#include "../../path_expention/path_exepension.h"
#include "../../error/error.h"
#include "../../memory/memory.h"
#include "../../data_structures/array_list.h"
#include "../../data_structures/data_string.h"
#include "../../variables/expand_special_variables.h"
#include "../../path_expention/path_exepension.h"

bool g_have_to_stop = 0; //to break in case of signal

#define IFS " \t\n"

static char *expand(char **to_expand);
char *scan_for_expand(char *line, bool is_quote);
static char *expand_cmd(char *to_expand, char to_stop, int nb_to_skip);

extern int get_nb_params(char **params)
{
    int res = 0;

    for (int i = 0; params[i]; i++)
    {
        res++;
    }

    return res;
}

static char *expand_tilde(char *str)
{
    if (strcmp("~", str) == 0)
        return strdup(getenv("HOME"));
    if (strcmp("~+", str) == 0)
        return get_current_dir_name();
    if (strcmp("~-", str) == 0)
        return strdup(g_env.old_pwd);
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


//to really understand take this example $(echo $(echo ok))
static char *expand_nested_command(char *cursor, char *to_expand,
                                                        char char_stopped_on)
{
    char *result;
    if (char_stopped_on == '$')
        result = expand_cmd(cursor, ')', 2);
    else
        result = expand_cmd(cursor, '`', 1);
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

static char *le_chapeau_de_expand_cmd(char **to_expand,
                                        char to_stop,
                                        int nb_to_skip
)
{
    char *beg = strdup(*to_expand);
    char *end_parenthesis = beg + nb_to_skip;
    find_corresponding_parenthesis(&end_parenthesis, NULL);
    *to_expand += (end_parenthesis - beg);
    *end_parenthesis = '\0';
    char *result = expand_cmd(beg, to_stop, nb_to_skip);
    free(beg);
    return result;
}

static char *expand_cmd(char *to_expand, char to_stop, int nb_to_skip)
{
    bool to_free = false;
    to_expand += nb_to_skip;

    char *cursor = to_expand;
    while ((cursor = strpbrk(cursor, "`$)\"\'\\")) != NULL
                                                        && *cursor != to_stop)
    {
        if (*cursor == '\"' || *cursor == '\'' || *cursor == '\\')
            skip_quoting(&cursor, NULL);
        else
        {
            //recursive call to expand command
            if ((*cursor == '$' && cursor[1] == '(') || *cursor == '`')
            {
                to_expand = expand_nested_command(cursor, to_expand, *cursor);
                cursor = to_expand;
                to_free = true;
            }
            else    //if we fall on a $ for a var we need to skip it
                cursor++;
        }
    }

    *cursor = '\0'; //replace ) with 0

    char *result = get_result_from_42sh(to_expand);
    if (to_free) //inner expansion that needs to be freed
        free(to_expand);
    return result;
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

    (*to_expand)++; //skip $
    char *value;

    char *beg = *to_expand;
    *to_expand = strpbrk(*to_expand, "$\'\"\\\n");
    char save;
    if (*to_expand != NULL)
    {
         save = **to_expand;
         **to_expand = '\0';
    }

    if ((value = hash_find(g_env.variables, beg)) == NULL)
        value = "";

    if (*to_expand != NULL)
        **to_expand = save;

    //if null, want to go to end of line
    if(*to_expand == NULL)
        *to_expand = beg + strlen(beg);

    //case we jump to next char and ++ in scan will skip it so scan goes onto it
    (*to_expand)--;
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

    *to_expand = (*to_expand + i + 1);
    return result;
}

static bool is_to_expand(char c, bool is_quote)
{
    return c == '$' || c == '\'' || c == '"' || c == '`'
                                                || (c == '\\' && !is_quote);
}

char *scan_for_expand(char *line, bool is_quote)
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
        if (is_to_expand(*line, is_quote))
        {
            char *expansion = expand(&line);
            string_append(new_line, expansion);
            free(expansion);
        }
        else
            string_append_char(new_line, *line);
        if (*line == '\0')
            break;
    }
    return string_get_content(&new_line);
}

char *expand_quote(char **cursor)
{
    if (**cursor == '\'')
    {
        (*cursor)++;
        char *beg = *cursor;
        *cursor = get_delimiter(*cursor, "\'");
        **cursor = '\0'; //set ' to 0
        *cursor = *cursor + 1;
        return strdup(beg);
    }
    else if (**cursor == '"')
    {
        (*cursor)++;
        char *beg = *cursor;
        *cursor = get_delimiter(*cursor, "\"\\");
        while (**cursor != '\"')
        {
            // Handle backslash
            if (**cursor == '\\')
                *cursor += 2;
            *cursor = get_delimiter(*cursor, "\"\\");
        }
        **cursor = '\0'; //set " to 0
        *cursor = *cursor + 1;
        return scan_for_expand(beg, true);
    }
    else // \ handling
    {
        (*cursor)++;
        *cursor = *cursor + 1;
        return strndup(*cursor, 1);
    }
}

static bool is_tidle(char *str)
{
    return strcmp(str, "~") == 0 || strcmp(str, "~+") == 0
                                                    || strcmp(str, "~-") == 0;
}

static char *expand(char **to_expand)
{
    if (is_tidle(*to_expand))
        return expand_tilde(*to_expand);
    if (**to_expand == '\'' || **to_expand == '"' || **to_expand == '\\')
        return expand_quote(to_expand);
    if (**to_expand == '$' && *(*to_expand + 1) == '(')
        return le_chapeau_de_expand_cmd(to_expand, ')', 2);
    if (**to_expand == '$' && *(*to_expand + 1) == '{')
        return expand_variable_brackets(to_expand);
    if (**to_expand == '$')
        return expand_variable(to_expand);
    if (**to_expand == '`')
        return le_chapeau_de_expand_cmd(to_expand, '`', 1);

    //strdup cause need to be able to free anything from expand
    return strdup(*to_expand); //no expansion
}

static void insert_sub_var(struct array_list *expanded_parameters,
                                            char *expansion
)
{
    char *beg = expansion; //cause strtok_r will make expansion move
    if (!is_multiple_words(expansion)) //expansion only gave one word
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

static int handle_expand_command(struct instruction *command_i)
{
    struct command_container *command = command_i->data;
    struct array_list *expanded_parameters = array_list_init();

    char *expansion = scan_for_expand(command->command, false);
    if (*expansion == '\0') //expand empty var
    {
        free(expansion);
        size_t i = 1;
        while (command->params[i] != NULL) //while empty var we remove
        {
            expansion = scan_for_expand(command->params[i], false);
            if (*expansion == '\0')
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
        expansion = scan_for_expand(command->params[i], false);
        if (*expansion == '\0') //empty var
        {
            free(expansion);
            continue;
        }
        else
        {
            if (!is_first) //the first param being command already pushed
                insert_sub_var(expanded_parameters, expansion);
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

void handle_sigint(int signal)
{
    if (signal == SIGINT)
    {
        puts("");
        g_have_to_stop = true;
    }
}

static int handle_if(struct instruction *ast)
{
    struct if_instruction *if_struct = ast->data;

    if (execute_ast(if_struct->conditions) == 0)
    {
        struct instruction *to_execute = if_struct->to_execute;
        return execute_ast(to_execute);
    }

    struct instruction *else_clause = if_struct->else_container;
    return execute_ast(else_clause);
}

static int handle_and_or_instruction(struct instruction *ast)
{
    struct and_or_instruction *node = ast->data;
    int return_code;
    if (ast->type == TOKEN_OR)
    {
        if ((return_code = execute_ast(node->left)) == 0
                || (return_code = execute_ast(node->right)) == 0)
            return 0;
        return return_code;
    }

    if ((return_code = execute_ast(node->left)) == 0
            && (return_code = execute_ast(node->right)) == 0)
        return 0;
    return return_code;
}

static bool is_func(struct instruction *ast)
{
    struct command_container *command = ast->data;
    if (command == NULL) //var assignement case
        return false;
    return hash_find(g_env.functions, command->command) != NULL;
}

static int exec_func(struct instruction *ast)
{
    struct command_container *command = ast->data;
    struct instruction *code = hash_find(g_env.functions, command->command);
    return execute_ast(code);
}

static bool is_builtin(struct instruction *ast)
{
    struct command_container *command = ast->data;
    if (command == NULL) //var assignement case
        return false;
    return hash_find_builtin(g_env.builtins, command->command) != NULL;
}

//for now only execute shopt
static int exec_builtin(struct instruction *ast)
{
    struct command_container *command = ast->data;
    int (*builtin)(char*[]) = hash_find_builtin(g_env.builtins,
                                                    command->command);

    int return_value;

    return_value = builtin(command->params);
    return return_value;
}

static struct command_container *dup_cmd(struct command_container *cmd);

static int handle_commands(struct instruction *ast)
{
    struct command_container *save = dup_cmd(ast->data);
    int to_return;

    if (handle_expand_command(ast) == -1)
        return 0;

    /* execute commande with zak function */
    if (is_func(ast))
        to_return = exec_func(ast);
    else if (is_builtin(ast))
        to_return = exec_builtin(ast);
    else
        to_return = exec_cmd(ast);

    struct command_container *cmd_c = ast->data;
    command_destroy(&cmd_c);
    ast->data = save;
    return to_return;
}


static int handle_while(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 0;
    g_env.is_in_loop = 1;

    while (!g_have_to_stop && execute_ast(while_instruction->conditions) == 0)
    {
        return_value = execute_ast(while_instruction->to_execute);

        if (g_env.breaks > 0)
        {
            g_env.breaks--;
            break;
        }

        if (g_env.continues > 0)
        {
            g_env.continues--;
            continue;
        }
    }

    g_have_to_stop = false;
    g_env.is_in_loop = 0;
    return return_value;
}


static int handle_until(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 0;
    g_env.is_in_loop = 1;

    while (!g_have_to_stop && execute_ast(while_instruction->conditions))
    {
        return_value = execute_ast(while_instruction->to_execute);

        if (g_env.breaks > 0)
        {
            g_env.breaks--;
            break;
        }

        if (g_env.continues > 0)
        {
            g_env.continues--;
            continue;
        }

    }

    g_have_to_stop = false;
    g_env.is_in_loop = 0;
    return return_value;
}


static struct command_container *dup_cmd(struct command_container *cmd)
{
    struct command_container *dup = xmalloc(sizeof(struct command_container));
    dup->command = strdup(cmd->command);

    size_t i = 0;
    char **new_params = xmalloc(sizeof(char*) * (get_nb_params(cmd->params)
                                                            + 2));

    while (1)
    {
        if (!cmd->params[i])
        {
            new_params[i] = NULL;
            break;
        }

        new_params[i] = strdup(cmd->params[i]);
        i++;
    }

    dup->params = new_params;
    return dup;
}


static int handle_for(struct instruction *ast)
{
    struct for_instruction *instruction_for = ast->data;
    struct array_list *var_values = instruction_for->var_values;
    int return_value;

    g_env.is_in_loop = 1;

    if (!var_values)
        return 0;

    for (size_t i = 0; i < var_values->nb_element && !g_have_to_stop; i++)
    {
        struct path_globbing *glob = sh_glob(var_values->content[i]);

        if (!glob)
        {
            hash_insert(g_env.variables, instruction_for->var_name,
                    var_values->content[i], STRING);

            return_value = execute_ast(instruction_for->to_execute);

            if (g_env.breaks > 0)
            {
                g_env.breaks--;
                break;
            }
            continue;
        }

        for (int j = 0; j < glob->nb_matchs; j++)
        {
            hash_insert(g_env.variables, instruction_for->var_name,
                    glob->matches->content[j], STRING);

            return_value = execute_ast(instruction_for->to_execute);

            if (g_env.breaks > 0)
            {
                break;
            }

            if (g_env.continues > 0)
            {
                g_env.continues--;
                continue;
            }
        }

        if (g_env.breaks > 0)
        {
            g_env.breaks--;
            break;
        }

        destroy_path_glob(glob);
    }

    g_env.is_in_loop = 0;
    return return_value;
}


static int handle_pipe(struct instruction *ast)
{
    struct pipe_instruction *pipe_instruction = ast->data;

    int fd[2];
    if (pipe(fd) == -1)
        return -1;

    int left;
    int right;
    if ((left = fork()) == 0)
    {
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        exit(execute_ast(pipe_instruction->left));
    }
    if ((right = fork()) == 0)
    {
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        exit(execute_ast(pipe_instruction->right));
    }
    close(fd[0]);
    close(fd[1]);

    int left_status;
    int right_status;
    waitpid(left, &left_status, 0);
    waitpid(right, &right_status, 0);
    return WEXITSTATUS(right_status);
}

static int check_patterns(char *pattern, struct array_list *patterns)
{
    for (size_t i = 0; i < patterns->nb_element; i++)
    {
        char *expantion = scan_for_expand(patterns->content[i], false);

        if (patterns->content[i] != expantion)
        {
            free(patterns->content[i]);
            patterns->content[i] = expantion;
        }

        if (fnmatch(patterns->content[i], pattern, FNM_EXTMATCH) == 0)
            return 1;
    }

    return 0;
}


static int handle_case(struct instruction *ast)
{
    struct case_clause *case_clause = ast->data;
    char *expantion = scan_for_expand(case_clause->pattern, false);

    if (case_clause->pattern != expantion)
    {
        free(case_clause->pattern);
        case_clause->pattern = expantion;
    }

    for (size_t i = 0; i < case_clause->items->nb_element; i++)
    {
        struct case_item *curr_item = case_clause->items->content[i];
        if (check_patterns(case_clause->pattern, curr_item->patterns))
            return execute_ast(curr_item->to_execute);
    }

    return 0;
}

extern int execute_ast(struct instruction *ast)
{
    if (!ast)
        return 0;

    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        errx(1, "an error occurred while setting up a signal handler");

    int return_value = 0;

    switch (ast->type)
    {
    case TOKEN_OR:
    case TOKEN_AND:
        return_value = handle_and_or_instruction(ast);
        break;
    case TOKEN_COMMAND:
        return_value = handle_commands(ast);
        break;
    case TOKEN_NOT:
        return_value = !(handle_commands(ast));
        break;
    case TOKEN_IF:
        return_value = handle_if(ast);
        break;
    case TOKEN_PIPE:
        return_value = handle_pipe(ast);
        break;
    case TOKEN_REDIRECT_LEFT:
    case TOKEN_REDIRECT_RIGHT:
    case TOKEN_REDIRECT_APPEND_LEFT:
    case TOKEN_REDIRECT_LEFT_TO_FD:
    case TOKEN_REDIRECT_READ_WRITE:
    case TOKEN_DUP_FD:
    case TOKEN_HEREDOC:
    case TOKEN_HEREDOC_MINUS:
    case TOKEN_OVERWRITE:
        return_value = redirections_handling(ast, 1);
        break;
    case TOKEN_WHILE:
        return_value = handle_while(ast);
        break;
    case TOKEN_UNTIL:
        return_value = handle_until(ast);
        break;
    case TOKEN_FOR:
        return_value = handle_for(ast);
        break;
    case TOKEN_CASE:
        return_value = handle_case(ast);
        break;
    default:
        return_value = 0;
    }

    g_env.last_return_value = return_value;
    if (ast->next != NULL && !g_env.breaks && !g_env.continues)
    {
        return_value = execute_ast(ast->next);
    }
    return return_value;
}
