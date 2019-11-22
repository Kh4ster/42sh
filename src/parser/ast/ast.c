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

#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"
#include "../../redirections_handling/redirect.h"
#include "../../data_structures/hash_map.h"
#include "../../input_output/get_next_line.h"

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
        execute_ast(to_execute);
        return 0;
    }

    struct instruction *else_clause = if_struct->else_container;
    execute_ast(else_clause);
    return 0;
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
    /* execute commande with zak function */
    if (is_func(ast))
        return exec_func(ast);
    else if (is_builtin(ast))
        return exec_builtin(ast);
    struct command_container *command = ast->data;
    return exec_cmd(command);
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
    int return_value = 1;

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

    for (size_t i = 0; i < var_values->nb_element; i++)
    {
        // assigne_variable(instruction_for->var_name, var_values->content[i]);
        return_value = execute_ast(instruction_for->to_execute);
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
        if (fnmatch(patterns->content[i], pattern, FNM_EXTMATCH) == 0)
            return 1;
    }

    return 0;
}


static int handle_case(struct instruction *ast)
{
    struct case_clause *case_clause = ast->data;

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
    if (!ast || ast->data == NULL)//for now to handle var assignement
        return 0;

    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        errx(1, "an error occurred while setting up a signal handler");

    int return_value;

    switch (ast->type)
    {
    case TOKEN_OR:
    case TOKEN_AND:
        return_value = handle_and_or_instruction(ast);
        break;
    case TOKEN_COMMAND:
        return_value = handle_commands(ast);
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
        return_value = redirections_handling(ast);
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

    if (ast->next != NULL)
        return_value = execute_ast(ast->next);

    return return_value;
}
