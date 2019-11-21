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

static void free_while(struct while_instruction *while_container)
{
    destroy_tree(while_container->conditions);
    destroy_tree(while_container->to_execute);
    free(while_container);
}

static void free_command(struct command_container *command)
{
    if (command == NULL) //FOR NOW FOR VAR ASSIGNEMENT, TO CHANGE
        return;
    free(command->command);

    for (size_t i = 0; command->params[i]; i++)
        free(command->params[i]);

    free(command->params);
    free(command);
}


static void free_redirection(struct redirection *redirection)
{
    free(redirection->file);
    destroy_tree(redirection->to_redirect);
    free(redirection);
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
        free(node);
        break;
    case TOKEN_COMMAND:
        free_command(ast->data);
        break;
    case TOKEN_IF:
        free_if(ast->data);
        break;
    case TOKEN_REDIRECT_APPEND_LEFT:
    case TOKEN_REDIRECT_LEFT:
    case TOKEN_REDIRECT_RIGHT:
    case TOKEN_REDIRECT_LEFT_TO_FD:
    case TOKEN_REDIRECT_READ_WRITE:
    case TOKEN_DUP_FD:
        free_redirection(ast->data);
        break;
    case TOKEN_WHILE:
    case TOKEN_UNTIL:
        free_while(ast->data);
        break;
    default:
        return;
    }

    destroy_tree(ast->next);

    free(ast);
}
