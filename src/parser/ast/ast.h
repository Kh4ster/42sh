#ifndef AST_H
#define AST_H

/** @file
* @brief abstact syntax tree structure
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

struct data_ast {
    enum token_parser_type type;
    char *test;
}


struct ast {
    struct data_ast *data;
    struct ast *left_child;
    struct ast *right_child;
};


extern struct ast *build_ast(struct stack *input);

#endif
