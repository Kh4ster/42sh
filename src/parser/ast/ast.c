#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"


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
        return execute_ast(node->left) == 0 || execute_ast(node->right) == 0;

    return execute_ast(node->left) == 0 && execute_ast(node->right) == 0;
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
        default:
            return 1;
    }
    return 1;
}

