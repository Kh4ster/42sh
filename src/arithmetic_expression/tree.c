#include <stdlib.h>

#include "tree.h"
#include "../data_structures/stack.h"

extern struct node *create_tree(struct stack *out)
{
    struct node *current = stack_pop(out);

    if (current->type == TOKEN_OPERAND)
        return current;

    current->data.operators->left_child = create_tree(out);
    current->data.operators->right_child = create_tree(out);

    return current;
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
