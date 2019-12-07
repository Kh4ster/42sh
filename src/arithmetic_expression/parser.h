/** @file
* @brief Code for the parser of arithmetic expressions.
* @author Coder : cloe.lacombe & pierrick Made
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once
#include "lexer.h"


enum type_node {
    AR_TOKEN_OPERATOR,
    AR_TOKEN_OPERAND
};


struct operators {
    enum token_type type;
    struct node *left_child;
    struct node *right_child;
};


union node_data {
    struct operators *operators;
    int data;
};


struct node {
    enum type_node type;
    union node_data data;
};


extern struct node *parser(char *line);

extern void destroy_arithmetic_tree(struct node *tree);
