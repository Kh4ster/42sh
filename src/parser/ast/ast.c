#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "../../execution_handling/command_execution.h"

static int handle_if(struct instruction *ast)
{
    struct if_instruction *if_struct = ast->data;
    if (execute_ast(if_struct->conditions))
    {
        execute_ast(if_struct->to_execute);
        return 1;
    }

    if (if_struct->elif_container)
        return execute_ast(if_struct->elif_container);

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
        default:
            return 1;
    }
    return 1;
}


extern void destroy_tree(struct instruction *ast)
{

