#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "parser.h"
#include "tree.h"
#include "../data_structures/stack.h"
#include "../memory/memory.h"

#define VALUE_PARENTHISIS 6

static struct node *create_node(enum type_node type, union node_data data)
{
    struct node *new_node = xmalloc(sizeof(struct node));
    new_node->type = type;
    new_node->data = data;
    return new_node;
}


static struct operators *create_operator(enum token_type type)
{
    struct operators *new_operator = xmalloc(sizeof(struct operators));
    new_operator->type = type;
    new_operator->left_child = NULL;
    new_operator->right_child = NULL;
    return new_operator;
}


static void push_operand_to_stack(struct stack *out, struct token *operand)
{
    union node_data data;
    data.data = atoi(operand->data);
    token_free(&operand);

    stack_push(out, create_node(TOKEN_OPERAND, data));
}


static void move_stack(struct stack *operators_stack, struct stack *out,
                                        struct token *operators, int nb_par)
{
    while (size_stack(operators_stack))
    {
        struct token *head_stack = stack_peek(operators_stack);

        if (!operators || head_stack->priority
                        > (operators->priority + nb_par * VALUE_PARENTHISIS))
        {
            head_stack = stack_pop(operators_stack);

            union node_data new_data;
            new_data.operators = create_operator(head_stack->type);
            stack_push(out, create_node(TOKEN_OPERATOR, new_data));
        }
    }

    stack_push(operators_stack, operators);
}


static int handle_parenthisis(char c, int *nb_parenthesis)
{
    if (c == '(')
        (*nb_parenthesis)++;

    else if (c == ')')
    {
        if (!nb_parenthesis)
        {
            return 1;
        }

        (*nb_parenthesis)--;
    }

    return 0;
}

extern struct node *parser(char *line)
{
    struct stack *out = init_stack();
    struct stack *operators = init_stack();
    struct token *current_token;
    int nb_parenthesis = 0;

    while (*line && ((current_token = token_get_next(&line)) != NULL))
    {
        if (*line == ')' || *line == '(')
        {
            if (handle_parenthisis(*line, &nb_parenthesis) == 1)
            {
                warnx("Bad parsing: wrong parenthesis");
                //TODO: stack destroy
                return NULL;
            }

            line++;
            continue;
        }

        if (current_token->type == TOKEN_PARAM)
            push_operand_to_stack(out, current_token);

        else
            move_stack(operators, out, current_token, nb_parenthesis);
    }

    if (nb_parenthesis)
    {
        warnx("Bad parsing: wrong parenthesis");
        //TODO: destroy stacks
        return NULL;
    }

    move_stack(operators, out, NULL, 0);
    return create_tree(out);
}

