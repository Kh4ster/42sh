#include <stdlib.h>
#include <math.h>

#include "tree.h"
#include "../data_structures/stack.h"

extern struct node *create_tree(struct stack *out)
{
    struct node *current = stack_pop(out);

    if (current->type == TOKEN_OPERAND)
        return current;

    if (current->data.operators->type == TOKEN_NOT ||
                    current->data.operators->type == TOKEN_BITWISE_NOT ||
                    current->data.operators->type == TOKEN_UNARY_MINUS)
        current->data.operators->left_child = create_tree(out);
    else
    {
        current->data.operators->right_child = create_tree(out);
        current->data.operators->left_child = create_tree(out);
    }

    return current;
}


extern int evaluate_tree(struct node *tree)
{
    if (tree->type == TOKEN_OPERAND)
        return tree->data.data;

    struct operators *data = tree->data.operators;

    switch (data->type)
        {
        case TOKEN_AND:
            return evaluate_tree(data->left_child) && evaluate_tree(data->right_child);
        case TOKEN_OR:
            return evaluate_tree(data->left_child) || evaluate_tree(data->right_child);
        case TOKEN_BITWISE_AND:
            return evaluate_tree(data->left_child) & evaluate_tree(data->right_child);
        case TOKEN_BITWISE_OR:
            return evaluate_tree(data->left_child) | evaluate_tree(data->right_child);
        case TOKEN_BITWISE_XOR:
            return evaluate_tree(data->left_child) ^ evaluate_tree(data->right_child);
        case TOKEN_BITWISE_NOT:
            return ~evaluate_tree(data->left_child);
        case TOKEN_PLUS:
            return evaluate_tree(data->left_child) + evaluate_tree(data->right_child);
        case TOKEN_MINUS:
            return evaluate_tree(data->left_child) - evaluate_tree(data->right_child);
        case TOKEN_MULTIPLY:
            return evaluate_tree(data->left_child) * evaluate_tree(data->right_child);
        case TOKEN_DIVIDE:
            return evaluate_tree(data->left_child) / evaluate_tree(data->right_child);
        case TOKEN_POWER:
            return pow(evaluate_tree(data->left_child), evaluate_tree(data->right_child));
        case TOKEN_NOT:
            return !evaluate_tree(data->left_child);
        case TOKEN_UNARY_MINUS:
            return -evaluate_tree(data->left_child);
        default:
            return 0;
        }
    return 0;
}

extern void destroy_tree(struct node *tree)
{
    if (tree->type == TOKEN_OPERAND)
    {
        free(tree);
        return;
    }

    destroy_tree(tree->data.operators->left_child);
    destroy_tree(tree->data.operators->right_child);

    free(tree);
}
