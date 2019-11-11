#ifndef AST_H
#define AST_H

/** @file
* @brief abstact syntax tree structure
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

struct ast {
    enum type_value type;
    struct ast *left_child;
    struct ast *right_child;
    char *test;
};



#endif
