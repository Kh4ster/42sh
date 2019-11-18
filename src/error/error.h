/** @file
* @brief Handling parsing errors
* @author Coder :
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../data_structures/queue.h"

/**
** @brief handle syntax/parsing errors
** @param lexer queue structure
*/
void handle_parser_errors(struct queue *lexer);
