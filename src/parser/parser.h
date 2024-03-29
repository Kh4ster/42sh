/** @file
* @brief module that will parse the token sent by the lexer
* @author Coder : nicolas.blin cloe.lacombe pierrick.made
* @author Tester : zakaria.ben-allal
* @author Reviewer : nicolas.blin cloe.lacombe pierrick.made
*/

#pragma once

#include <stdio.h>

#include "../data_structures/queue.h"

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
    TOKEN_DUP_FD, /**< @brief token fd duplication */
    TOKEN_OVERWRITE, /**< @brief token overwrite */
    TOKEN_COMMAND, /**< @brief token command */
    TOKEN_ELSE, /**< @brief token else */
    TOKEN_CASE, /**< @brief token case */
    TOKEN_HEREDOC, /**< @brief token here doc */
    TOKEN_NOT, /**< @brief token not */
    TOKEN_HEREDOC_MINUS, /**< @brief token heredoc minux */
    TOKEN_WHILE, /**< @brief token while */
    TOKEN_UNTIL, /**< @brief token until */
    TOKEN_FOR, /**< @brief token for */
    TOKEN_PIPE, /**< @brief token pipe */
    TOKEN_VAR_DECLARATION, /**< @brief token var declaration */
    TOKEN_FUNC_DEF /**< @brief token func definition */
};

/**
* @struct for_instruction
* @brief represent an for node in the ast
*/
struct for_instruction
{
    char *var_name;
    struct array_list *var_values;
    struct instruction *to_execute;
};

/**
* @struct case_item
* @brief represent an case item for the case clause
*/
struct case_item
{
    struct array_list *patterns; /**< @brief patterns to match */
    struct instruction *to_execute; /**< @brief commands to execute */
    struct case_item *next; /**< @brief next item */
};

/**
* @struct case_clause
* @brief represent a case node in the ast
*/
struct case_clause
{
    char *pattern; /**< @brief pattern to match */
    struct array_list *items; /**< @brief items */
};

/**
* @struct and_or_instruction
* @brief represent a while node in the ast
*/
struct while_instruction
{
    struct instruction *conditions; /**< @brief conditions to test */
    struct instruction *to_execute; /**< @brief commands to execute */
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
* @struct pipe_instruction
* @brief represent a pipe node in the ast
*/
struct pipe_instruction
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
    FILE *temp_file; /**< @brief only for heredocs */
};

/**
* @brief apply the grammar rule on the input
* @param lexer the lexer that we will use for our pop/peek
* @return success : an ast, fail : NULL
*/
struct instruction *parse_input(struct queue *lexer, int *is_end, int *error);

/**
* @brief build an instruction (exported for unit test)
* @param type type of the data behind the void*
* @param input_instr the instruction
* @return return an instruction
*/
struct instruction *build_instruction(enum token_parser_type type,
                                                            void *input_instr);
