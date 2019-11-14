/** @file
* @brief
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#include "../data_structures/queue.h"

#pragma once

enum token_parser_type
{
    TOKEN_IF = 0,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_REDIRECT_LEFT,
    TOKEN_REDIRECT_RIGHT,
    TOKEN_REDIRECT_APPEND_LEFT,
    TOKEN_COMMAND,
    TOKEN_ELSE
};

struct and_or_instruction
{
    struct instruction *left;
    struct instruction *right;
};

struct if_instruction
{
    struct instruction *conditions; /**< @brief conditions to test */
    struct instruction *to_execute; /**< @brief commands to execute */
    struct instruction *elif_container; /**< @brief elif (can be null)*/
    struct instruction *else_container; /**< @brief else (can be null) */
};

struct instruction
{
    void *data;
    enum token_parser_type type;
};

struct instruction* parse_input(struct queue *lexer);