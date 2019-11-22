#include <stdlib.h>

#include "ast.h"
#include "../parser.h"
#include "../../execution_handling/command_container.h"
#include "destroy_tree.h"
#include "../../data_structures/array_list.h"

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

static void free_for(struct for_instruction *for_instruction)
{
    if (for_instruction->var_values)
        array_list_destroy(for_instruction->var_values);

    free(for_instruction->var_name);
    destroy_tree(for_instruction->to_execute);
    free(for_instruction);
}

extern void free_case_item(struct case_item *item)
{
    array_list_destroy(item->patterns);
    destroy_tree(item->to_execute);
}


extern void free_case_clause(struct case_clause *clause)
{
    for (size_t i = 0; i < clause->items->nb_element; i++)
        free_case_item(clause->items->content[i]);

    array_list_destroy(clause->items);
    free(clause->pattern);
    free(clause);
}


extern void destroy_tree(struct instruction *ast)
{
    if (!ast)
        return;

    struct and_or_instruction *node = NULL;
    struct pipe_instruction *pipe = NULL;

    switch (ast->type)
    {
    case TOKEN_OR:
    case TOKEN_AND:
        node = ast->data;
        destroy_tree(node->left);
        destroy_tree(node->right);
        free(node);
        break;
    case TOKEN_PIPE:
        pipe = ast->data;
        destroy_tree(pipe->left);
        destroy_tree(pipe->right);
        free(pipe);
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
    case TOKEN_FOR:
        free_for(ast->data);
    case TOKEN_CASE:
        free_case_clause(ast->data);
        break;
    default:
        return;
    }

    destroy_tree(ast->next);

    free(ast);
}
