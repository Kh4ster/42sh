#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"
#include "../../redirections_handling/redirect.h"


static int handle_if(struct instruction *ast)
{
    struct if_instruction *if_struct = ast->data;

    if (execute_ast(if_struct->conditions))
    {
        struct instruction *to_execute = if_struct->to_execute;

        for (; to_execute; to_execute = to_execute->next)
            execute_ast(to_execute);

        return 1;
    }

    return execute_ast(if_struct->else_container);
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
    return exec_cmd(command);
}


extern int execute_ast(struct instruction *ast)
{
    if (!ast)
        return 1;

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
            return redirections_handling(ast->data);
            break;
        default:
            return 1;
    }
    return 1;
}

