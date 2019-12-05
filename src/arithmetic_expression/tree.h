/** @file
* @brief File that holds every necessary code for the arithmetic tree
* @author Coder : cloe.lacombe & pierrick.made
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once
#include "parser.h"
#include "../data_structures/stack.h"

extern struct node *create_tree(struct stack *out);

extern int evaluate_tree(struct node *tree);

extern void destroy_ar_tree(struct node *tree);
