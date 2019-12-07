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
* @brief return the number of params
* @param params the list to count
* @return number of param
*/
extern int get_nb_params(char **params);

/**
* @brief function that execute the ast depending on the type of the token
* @param ast the root of the tree being executed
* @return value : return the return code of the tree (0 if success, else errno is the return code)
*/
extern int execute_ast(struct instruction *ast);

/**
* @brief function that scan a word and performs expansion
* @details eveything is stored in a string, the cursor moves to expand everyting
* @param line the line to scan
* @param is_quote know if we are comming from a recusive call with quote
* @param was_quote so that callers knows is there was quote
* @return a string with all expansion performed on the string
*/
char *scan_for_expand(char *line, bool is_quote, int *was_quote);

/**
* @brief perform one expansion $, `, "...
* @param to_expand expand move the to_expand pointer
* @details for example ...$(...)... will move from $ to )
* @param is_quote know if we are comming from a recusive call with quote
* @param was_quote so that callers knows is there was quote
* @return a string with all expansion performed on the string
*/
char *expand(char **to_expand, bool is_quote, int *was_quote);