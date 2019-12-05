#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "parser.h"
#include "tree.h"
#include "../data_structures/stack.h"
#include "../memory/memory.h"

#define VALUE_PARENTHISIS 10

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

    stack_push(out, create_node(AR_TOKEN_OPERAND, data));
    token_free(&operand);
}


static void move_stack(struct stack *operators_stack, struct stack *out,
                                        struct token *operators, int nb_par)
{
    if (operators)
        operators->priority += nb_par * VALUE_PARENTHISIS;

    while (size_stack(operators_stack))
    {
        struct token *head_stack = stack_peek(operators_stack);

        if (!operators || head_stack->priority > operators->priority)
        {
            head_stack = stack_pop(operators_stack);

            union node_data new_data;
            new_data.operators = create_operator(head_stack->type);
            stack_push(out, create_node(AR_TOKEN_OPERATOR, new_data));
            token_free(&head_stack);
        }
        else
            break;
    }

    if (operators)
        stack_push(operators_stack, operators);
}


static int handle_parenthisis(enum token_type type, int *nb_parenthesis)
{
    if (type == AR_TOKEN_LEFT_PARENTHESIS)
        (*nb_parenthesis)++;

    else if (type == AR_TOKEN_RIGHT_PARENTHESIS)
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
    int old_type = AR_TOKEN_PLUS;

    while (line && *line && ((current_token = token_get_next(&line)) != NULL))
    {
        if (current_token->type == AR_TOKEN_LEFT_PARENTHESIS ||
            current_token->type == AR_TOKEN_RIGHT_PARENTHESIS)
        {
            if (handle_parenthisis(current_token->type, &nb_parenthesis) == 1)
            {
                token_free(&current_token);
                warnx("Bad parsing: wrong parenthesis");
                //TODO: stack destroy
                return NULL;
            }

            token_free(&current_token);
            continue;
        }

        if (current_token->type == AR_TOKEN_PARAM)
        {
            push_operand_to_stack(out, current_token);
            old_type = AR_TOKEN_PARAM;
        }
        else
        {
            if (old_type != AR_TOKEN_PARAM)
            {
                if (current_token->type == AR_TOKEN_MINUS)
                {
                    current_token->type = AR_TOKEN_UNARY_MINUS;
                    current_token->priority = 9;
                }
            }

            move_stack(operators, out, current_token, nb_parenthesis);
            old_type = current_token->type;
        }

        //token_free(&current_token);
    }

    if (nb_parenthesis)
    {
        warnx("Bad parsing: wrong parenthesis");
        //TODO: destroy stacks
        return NULL;
    }

    move_stack(operators, out, NULL, 0);
    free(operators);
    struct node *to_return = create_tree(out);
    free(out);
    return to_return;
}
