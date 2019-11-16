/** @file
* @brief abstact syntax tree structure
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#ifndef AST_H
#define AST_H

#include "../parser.h"

/**
* @brief function that execute the ast depending on the type of the token
* @param ast: the root of the tree being executed
* @return value : return the return code of the tree (0 if success, else errno is the return code)
*/
extern int execute_ast(struct instruction *ast);

#endif
