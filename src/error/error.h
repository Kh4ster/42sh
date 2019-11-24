/** @file
* @brief Handling parsing errors
*/

#pragma once

#include "../data_structures/queue.h"

/**
* @brief handle syntax/parsing errors
* @param lexer queue structure
*/
void handle_parser_errors(struct queue *lexer);
