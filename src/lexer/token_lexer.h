/** @file
* @brief Data_structure of token used and returned by lexer
* @author Coder : pierrick.made
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../input_output/get_next_line.h"
#include "../data_structures/queue.h"

/** @enum token_lexer_type
* @brief Possible types of a token generated by the lexer
*/
enum token_lexer_type
{
    TOKEN_END_OF_INSTRUCTION = 1, /**< @brief end of an instruction ";" */
    TOKEN_EOF, /**< @brief end of file "\n" */
    TOKEN_OPERATOR, /**< @brief operators defined in scl (&&, ||, ;;, ...) */
    TOKEN_KEYWORD, /**< @brief reserved words like fi, then, ... */
    TOKEN_OTHER, /**< @brief any words without delimiters */
    TOKEN_ASSIGNEMENT /**< @brief variable assignement */
};

/** @struct token_lexer
* @brief Define a token generated by the lexer
*/
struct token_lexer
{
    char *data; /**< @brief token content (char*) */
    enum token_lexer_type type; /**< @brief type of the token */
};

/**
* @brief Add all token_lexer found in line to token_queue
* @param line The line to be divided into tokens by the lexer
* @param token_queue current token queue to fill with line's tokens
* @return success : token_queue, fail : NULL
* @relates token_lexer
*/
struct queue *lexer(char *line, struct queue *token_queue);

struct token_lexer *token_lexer_head(struct queue *token_queue);

struct token_lexer *token_lexer_pop(struct queue *token_queue);

/**
* @brief Free the token struct and contained string (set token to NULL)
* @param token the token you want to free
* @relates token_lexer
*/
void token_lexer_free(struct token_lexer **token);

/**
* @brief Free the queue struct and all its tokens
* @param token the token queue you want to free
* @relates token_lexer
*/
void token_queue_free(struct queue **token_queue);