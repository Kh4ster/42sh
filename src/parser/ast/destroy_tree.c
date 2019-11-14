#include <stdlib.h>

#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "destroy_tree.h"

static void free_if (struct if_instruction *if_container)
{
    destroy_tree(if_container->conditions);
    destroy_tree(if_container->to_execute);
    destroy_tree(if_container->else_container);
    free(if_container);
}


static void free_command(struct command_container *command)
{
    free(command->command);

    for (size_t i = 0; command->params[i]; i++)
        free(command->params[i]);

    free(command->params);
    free(command);
}


extern void destroy_tree(struct instruction *ast)
{
    if (!ast)
        return;

    struct and_or_instruction *node;
    switch (ast->type)
    {
        case TOKEN_OR:
        case TOKEN_AND:
            node = ast->data;
            destroy_tree(node->left);
            destroy_tree(node->right);
            break;
        case TOKEN_COMMAND:
            free_command(ast->data);
            break;
        case TOKEN_IF:
            free_if(ast->data);
            break;
        default:
            return;
    }
    free(ast);
}
