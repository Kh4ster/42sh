#include <signal.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>

#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"
#include "../../redirections_handling/redirect.h"

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

    if (ast->type == TOKEN_OR)
    {
        if (execute_ast(node->left) == 0 || execute_ast(node->right) == 0)
            return 0;
        return 1;
    }

    if (execute_ast(node->left) == 0 && execute_ast(node->right) == 0)
        return 0;
    return 1;
}


static int handle_commands(struct instruction *ast)
{
    /* execute commande with zak function */
    struct command_container *command = ast->data;
    return exec_cmd(command);
}


static int handle_while(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 1;

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


extern int execute_ast(struct instruction *ast)
{
    if (!ast || ast->data == NULL)//for now to handle var assignement
        return 1;

    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        errx(1, "an error occured while setting up a signal handler");

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
    case TOKEN_REDIRECT_LEFT:
    case TOKEN_REDIRECT_RIGHT:
    case TOKEN_REDIRECT_APPEND_LEFT:
        return_value = redirections_handling(ast);
        break;
    case TOKEN_WHILE:
        return_value = handle_while(ast);
        break;
    case TOKEN_UNTIL:
        return_value = handle_until(ast);
        break;
    default:
        return_value = 1;
    }

    if (ast->next != NULL)
        return_value = execute_ast(ast->next);
    return return_value;
}