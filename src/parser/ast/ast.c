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
#include "../../path_expention/path_exepension.h"
#include "../../error/error.h"
#include "../../memory/memory.h"
#include "../../data_structures/array_list.h"

bool g_have_to_stop = 0; //to break in case of signal

static char *expand(char *to_expand);


static void expand_tilde_in_params(char **params)
{
    for (int i = 0; params[i]; i++)
    {
        if (strcmp("~", params[i]) == 0)
        {
            free(params[i]);
            params[i] = strdup(getenv("HOME"));
        }

        if (strcmp("~+", params[i]) == 0)
        {
            free(params[i]);
            params[i] = get_current_dir_name();
        }

        if (strcmp("~-", params[i]) == 0)
        {
            free(params[i]);
            params[i] = strdup(getenv("OLDPWD"));
        }
    }
}

static void expand_tilde(struct command_container *cmd)
{
    if (strcmp("~", cmd->command) == 0)
    {
        free(cmd->command);
        cmd->command = strdup(getenv("HOME"));
    }

    if (strcmp("~+", cmd->command) == 0)
    {
        free(cmd->command);
        cmd->command = get_current_dir_name();
    }

    if (strcmp("~-", cmd->command) == 0)
    {
        free(cmd->command);
        cmd->command = strdup(getenv("OLDPWD"));
    }
    expand_tilde_in_params(cmd->params);
}

static struct command_container *build_cmd(struct array_list *params,
                                                char **params_cmd)
{
    struct array_list *list = array_list_init();

    for (size_t i = 1; i < params->nb_element; i++)
        array_list_append(list, strdup(params->content[i]));

    for (size_t i = 0; params_cmd[i]; i++)
        array_list_append(list, strdup(params_cmd[i]));

    struct command_container *cmd = command_create(params->content[0], list);
    free(list->content);
    free(list);
    return cmd;
}


static void expand_glob_cmd(struct instruction *cmd_i)
{
    struct command_container *cmd = cmd_i->data;
    struct path_globbing *glob = sh_glob(cmd->command);

    if (!glob)
        return;

    cmd_i->data = build_cmd(glob->matches, cmd->params);
    command_destroy(&cmd);
    destroy_path_glob(glob);
}


//to really understand take this example $(echo $(echo ok))
static char *expand_nested_command(char *cursor, char *to_expand)
{
    char *result = expand(cursor);
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
                to_expand = expand_nested_command(cursor, to_expand);
                cursor = to_expand;
                to_free = true;
            }
            else    //if we fall on a $ for a var we need to skip it
                cursor++;
        }
    }
    if (cursor == NULL)
        handle_parser_errors(NULL);
    *cursor = 0; //replace ) with 0

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
    free(command->command);
    command->command = strdup(strtok_r(expansion, " ", &expansion));
    //first parameter is command
    array_list_append(parameters, strdup(command->command));

    char *param;
    while ((param = strtok_r(NULL, " ", &expansion)) != NULL)
        array_list_append(parameters, strdup(param));
}

static bool is_multiple_words(char *expansion)
{
    return strpbrk(expansion, " ") != NULL;
}

static char *expand_variable(char *to_expand)
{
    to_expand++; //skip $
    char *value;

    if ((value = hash_find(g_env.variables, to_expand)) != NULL)
        return strdup(value);
    return strdup("");
}

static char *expand(char *to_expand)
{
    if (to_expand[0] == '$' && to_expand[1] == '(')
        to_expand = expand_cmd(to_expand, ')', 2);
    if (to_expand[0] == '$')
        to_expand = expand_variable(to_expand);
    if (to_expand[0] == '`')
        to_expand = expand_cmd(to_expand, '`', 1);

    return to_expand; //no expansion
}

static void insert_sub_var(struct array_list *expanded_parameters,
                                            char *expansion
)
{
    if (!is_multiple_words(expansion)) //expansion only gave one word
    {
        array_list_append(expanded_parameters, strdup(expansion));
        return;
    }

    //first call without NULL
    array_list_append(expanded_parameters, strdup(strtok_r(expansion, " ",
                                                                &expansion)));

    char *param;
    while ((param = strtok_r(NULL, " ", &expansion)) != NULL)
        array_list_append(expanded_parameters, strdup(param));
}

static int handle_expand_command(struct instruction *command_i)
{
    struct command_container *command = command_i->data;
    struct array_list *expanded_parameters = array_list_init();

    char *expansion = expand(command->command);
    if (*expansion == '\0') //expand empty var
    {
        free(expansion);
        size_t i = 1;
        while (command->params[i] != NULL) //while empty var we remove
        {
            expansion = expand(command->params[i]);
            if (*expansion == '\0')
            {
                free(expansion);
                i++;
            }
            else
                break;
        }
    }

    if (is_multiple_words(expansion)) //var="echo ok ok ..."
        fill_command_and_params(command, expansion, expanded_parameters);
    else
    {
        if (command->command != expansion) //if an expansion was performed
        {
            free(command->command);
            command->command = strdup(expansion);
        }
        array_list_append(expanded_parameters, strdup(command->command));
    }

    bool is_first = true;
    for (size_t i = 0; command->params[i] != NULL; ++i)
    {
        expansion = expand(command->params[i]);
        if (*expansion == '\0') //empty var
        {
            free(expansion);
            continue;
        }
        else
        {
            if (!is_first) //the first param being command alreay pushed
                insert_sub_var(expanded_parameters, expansion);
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

    expand_tilde(command_i->data);
    expand_glob_cmd(command_i);
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

static int handle_commands(struct instruction *ast)
{
    if (handle_expand_command(ast) == -1)
        return 0;

    /* execute commande with zak function */
    if (is_func(ast))
        return exec_func(ast);
    else if (is_builtin(ast))
        return exec_builtin(ast);

    return exec_cmd(ast);
}


static int handle_while(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 0;

    while (!g_have_to_stop && execute_ast(while_instruction->conditions) == 0)
        return_value = execute_ast(while_instruction->to_execute);

    g_have_to_stop = false;
    return return_value;
}


static int handle_until(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 0;

    while (!g_have_to_stop && execute_ast(while_instruction->conditions))
        return_value = execute_ast(while_instruction->to_execute);

    g_have_to_stop = false;
    return return_value;
}


static int handle_for(struct instruction *ast)
{
    struct for_instruction *instruction_for = ast->data;
    struct array_list *var_values = instruction_for->var_values;
    int return_value;

    if (!var_values)
        return 0;

    for (size_t i = 0; i < var_values->nb_element && !g_have_to_stop; i++)
    {
        struct path_globbing *glob = sh_glob(var_values->content[i]);
        // TODO assigne_variable(instruction_for->var_name, var_values->content[i]);

        if (!glob)
        {
            return_value = execute_ast(instruction_for->to_execute);
            continue;
        }

        for (int j = 0; j < glob->nb_matchs; j++)
        {
            // TODO asigne_variable(instruction_for->var_name, glob->matches->content[j]);
            return_value = execute_ast(instruction_for->to_execute);
        }
        destroy_path_glob(glob);
    }

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
        char *expantion = expand(patterns->content[i]);

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
    char *expantion= expand(case_clause->pattern);

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
    if (ast->next != NULL)
        return_value = execute_ast(ast->next);

    return return_value;
}