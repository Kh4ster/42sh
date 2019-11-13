#include "ast.h"
#include "../parser.h"
#include "../parser.h"

static int (struct if_instruction *if_struct)
{
    
}


static int handle_and_or_instruction(struct instruction *ast)
{
    struct and_or_instruction *node = ast->data;

    if (ast->type == OR)
        return execute_ast(node->left) || execute_ast(node->right);

    return execute_ast(node->left) && execute_ast(node->right);
}


static int handle_commands(struct instructions *ast)
{
    /* execute commande with zak function */
    return 1;
}


extern int execute_ast(struct instruction *ast)
{
    switch (ast->type)
    {
        case TOKEN_OR:
        case TOKEN_AND:
            return handle_and_or_instruction(ast);
            break;
        case TOKEN_COMMAND:
            return handle_commands(ast);
            break;
    }
    return 1;
}
