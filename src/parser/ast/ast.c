#include <signal.h>
#include <err.h>
#include <stdio.h>

#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"
#include "../../redirections_handling/redirect.h"

int have_to_stop = 0;

void handle_sigint(int signal)
{
    if (signal == SIGINT)
    {
        puts("");
        have_to_stop = 1;
    }
}

static int handle_if(struct instruction *ast)
{
    struct if_instruction *if_struct = ast->data;

    if (execute_ast(if_struct->conditions) == 0)
    {
        struct instruction *to_execute = if_struct->to_execute;

        for (; to_execute; to_execute = to_execute->next)
            execute_ast(to_execute);

        return 0;
    }

    execute_ast(if_struct->else_container);
    return 0;
}


static int handle_and_or_instruction(struct instruction *ast)
{
    struct and_or_instruction *node = ast->data;

    if (ast->type == TOKEN_OR)
        return execute_ast(node->left) || execute_ast(node->right);

    return execute_ast(node->left) && execute_ast(node->right);
}


static int handle_commands(struct instruction *ast)
{
    /* execute commande with zak function */
    struct command_container *command = ast->data;
    return exec_cmd(command) == 0;
}

static int handle_while(struct instruction *ast)
{
    struct while_instruction *while_instruction = ast->data;
    int return_value = 1;

    while (!have_to_stop && execute_ast(while_instruction->conditions))
        return_value = execute_ast(while_instruction->to_execute);

    return return_value;
}

extern int execute_ast(struct instruction *ast)
{
    if (!ast || ast->data == NULL)//for now to handle var assignement
        return 1;

    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        errx(1, "an error occured while setting up a signal handler");

    switch (ast->type)
    {
    case TOKEN_OR:
    case TOKEN_AND:
        return handle_and_or_instruction(ast);
        break;
    case TOKEN_COMMAND:
        return handle_commands(ast);
        break;
    case TOKEN_IF:
        return handle_if(ast);
        break;
    case TOKEN_REDIRECT_LEFT:
    case TOKEN_REDIRECT_RIGHT:
    case TOKEN_REDIRECT_APPEND_LEFT:
        return redirections_handling(ast);
        break;
    case TOKEN_WHILE:
        return handle_while(ast);
        break;
    default:
        return 1;
    }

    return 1;
}

