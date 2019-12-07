#include <stdlib.h>
#include <math.h>

#include "tree.h"
#include "../data_structures/stack.h"

extern struct node *create_tree(struct stack *out)
{
    struct node *current = stack_pop(out);

    if (!current)
        return NULL;

    if (current->type == AR_TOKEN_OPERAND)
        return current;

    if (current->data.operators->type == AR_TOKEN_NOT ||
                    current->data.operators->type == AR_TOKEN_BITWISE_NOT ||
                    current->data.operators->type == AR_TOKEN_UNARY_MINUS ||
                    current->data.operators->type == AR_TOKEN_UNARY_PLUS)
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
    if (tree->type == AR_TOKEN_OPERAND)
        return tree->data.data;

    struct operators *data = tree->data.operators;

    switch (data->type)
        {
        case AR_TOKEN_AND:
            return evaluate_tree(data->left_child) && evaluate_tree(data->right_child);
        case AR_TOKEN_OR:
            return evaluate_tree(data->left_child) || evaluate_tree(data->right_child);
        case AR_TOKEN_BITWISE_AND:
            return evaluate_tree(data->left_child) & evaluate_tree(data->right_child);
        case AR_TOKEN_BITWISE_OR:
            return evaluate_tree(data->left_child) | evaluate_tree(data->right_child);
        case AR_TOKEN_BITWISE_XOR:
            return evaluate_tree(data->left_child) ^ evaluate_tree(data->right_child);
        case AR_TOKEN_BITWISE_NOT:
            return ~evaluate_tree(data->left_child);
        case AR_TOKEN_PLUS:
            return evaluate_tree(data->left_child) + evaluate_tree(data->right_child);
        case AR_TOKEN_MINUS:
            return evaluate_tree(data->left_child) - evaluate_tree(data->right_child);
        case AR_TOKEN_MULTIPLY:
            return evaluate_tree(data->left_child) * evaluate_tree(data->right_child);
        case AR_TOKEN_DIVIDE:
            return evaluate_tree(data->left_child) / evaluate_tree(data->right_child);
        case AR_TOKEN_POWER:
            return pow(evaluate_tree(data->left_child), evaluate_tree(data->right_child));
        case AR_TOKEN_NOT:
            return !evaluate_tree(data->left_child);
        case AR_TOKEN_UNARY_MINUS:
            return -evaluate_tree(data->left_child);
        case AR_TOKEN_UNARY_PLUS:
            return evaluate_tree(data->left_child);
        default:
            return 0;
        }
    return 0;
}

extern void destroy_ar_tree(struct node *tree)
{
    if (!tree)
        return;

    if (tree->type == AR_TOKEN_OPERAND)
    {
        free(tree);
        return;
    }

    destroy_ar_tree(tree->data.operators->left_child);
    destroy_ar_tree(tree->data.operators->right_child);

    free(tree->data.operators);
    free(tree);
}
