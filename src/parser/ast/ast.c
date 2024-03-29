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
#include "../../special_variables/expand_special_variables.h"
#include "../../path_expention/path_exepension.h"
#include "../../arithmetic_expression/parser.h"
#include "../../arithmetic_expression/tree.h"
#include "../../command_substitution/command_substitution.h"
#include "../../command_expansion/command_expansion.h"

bool g_have_to_stop = 0; //to break in case of signal

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
    int old_argc = g_env.argc;
    char **old_argv = g_env.argv;
    g_env.argc = get_nb_params(command->params) - 1;
    g_env.argv = command->params;
    struct instruction *code = hash_find(g_env.functions, command->command);
    int to_return = execute_ast(code);
    g_env.argc = old_argc;
    g_env.argv = old_argv;
    return to_return;

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
    {
        command_destroy(&save);
        return 0;
    }

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
    g_env.is_in_loop++;

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
    g_env.is_in_loop--;
    return return_value;
}


static int handle_until(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 0;
    g_env.is_in_loop++;

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
    g_env.is_in_loop--;
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


struct array_list *update_for_values(struct array_list *for_values)
{
    if (!for_values)
        return NULL;


    struct array_list *new = array_list_init();

    for (size_t i = 0; i < for_values->nb_element; i++)
    {
        char *to_expand = for_values->content[i];
        char *expand = scan_for_expand(to_expand, false, NULL);
        char *begin = expand;

        while (begin)
        {
            char *end = strchr(begin, ' ');

            if (end)
            {
                *end = '\0';
            }

            array_list_append(new, strdup(begin));

            if (end)
                begin = end + 1;
            else
                begin = NULL;
        }

        free(expand);
    }

    array_list_destroy(for_values);
    return new;
}


static int handle_for(struct instruction *ast)
{
    struct for_instruction *instruction_for = ast->data;
    instruction_for->var_values = update_for_values(instruction_for->var_values);
    struct array_list *var_values = instruction_for->var_values;
    int return_value;

    g_env.is_in_loop++;

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

            if (g_env.continues > 0)
            {
                g_env.continues--;
                if (g_env.continues > 0)
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
                if (g_env.continues > 0)
                    break;
                continue;
            }
        }

        if (g_env.breaks > 0)
        {
            g_env.breaks--;
            break;
        }

        if (g_env.continues > 0)
            break;

        destroy_path_glob(glob);
    }

    g_env.is_in_loop--;
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
        char *expantion = scan_for_expand(patterns->content[i], true, NULL);

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
    char *expantion = scan_for_expand(case_clause->pattern, true, NULL);

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
        return_value = execute_ast(ast->next);
    
    return return_value;
}
