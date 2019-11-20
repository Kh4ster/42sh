/** @file
* @brief
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#include "../data_structures/queue.h"

#pragma once

/**
* @enum token_parser_type
* @brief tokens that will be in the ast
*/
enum token_parser_type
{
    TOKEN_IF = 0, /**< @brief token if */
    TOKEN_OR, /**< @brief token or */
    TOKEN_AND, /**< @brief token and */
    TOKEN_REDIRECT_LEFT, /**< @brief token redirect left */
    TOKEN_REDIRECT_RIGHT, /**< @brief token redirect right */
    TOKEN_REDIRECT_APPEND_LEFT, /**< @brief token append left */
    TOKEN_REDIRECT_LEFT_TO_FD,
    TOKEN_REDIRECT_READ_WRITE,
    TOKEN_COMMAND, /**< @brief token command */
    TOKEN_ELSE, /**< @brief token else */
    TOKEN_DUP_FD
    TOKEN_WHILE,
    TOKEN_UNTIL,
    TOKEN_FOR
};


struct while_instruction
{
    struct instruction *conditions;
    struct instruction *to_execute;
};

/**
* @struct and_or_instruction
* @brief represent an "and" or "or" node in the ast
*/
struct and_or_instruction
{
    struct instruction *left; /**< @brief left child to execute */
    struct instruction *right; /**< @brief right child to execute */
};

/**
* @struct if_instruction
* @brief represent an if node in the ast
*/
struct if_instruction
{
    struct instruction *conditions; /**< @brief conditions to test */
    struct instruction *to_execute; /**< @brief commands to execute */
    struct instruction *else_container; /**< @brief elif/else **/
};

/**
* @struct instruction
* @brief the generic instruction node that may contain any of the above nodes
*/
struct instruction
{
    void *data; /**< @brief contains a ast node */
    enum token_parser_type type; /**< @brief type of ast node */
    struct instruction *next; /**< @brief use for ; cases */
};

/**
* @struct redirection
* @brief represent a redirection node in the ast
*/
struct redirection
{
    int fd; /**< @brief io number */
    char *file; /**< @brief file to use for redirection */
    struct instruction *to_redirect; /**< @brief command to execute */
};

/**
* @brief apply the grammar rule on the input
* @param lexer the lexer that we will use for our pop/peek
* @return success : an ast, fail : NULL
*/
struct instruction *parse_input(struct queue *lexer, int *is_end, int *error);
