/** @file
* @brief abstact syntax tree structure
* @author Coder : cloe.lacombe & nicolas.blin & pierrick.made
* @author Tester : zakaria.ben-allal
* @author Reviewer : nicolas.blin & pierrick.made
*/

#pragma once

#include <stdbool.h>

#include "../parser.h"

/**
* @brief function that execute the ast depending on the type of the token
* @param ast: the root of the tree being executed
* @return value : return the return code of the tree (0 if success, else errno is the return code)
*/

extern int get_nb_params(char **params);

extern int execute_ast(struct instruction *ast);