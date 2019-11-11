/** @file
* @brief 
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

enum token_parser_type
{
    TOKEN_IF = 0,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_REDIRECT_LEFT,
    TOKEN_REDIRECT_RIGHT,
    TOKEN_REDIRECT_APPEND_LEFT,
    TOKEN_INSTRUCTION,
    TOKEN_ELSE
};

enum token_lexer_to_ast_type
{
    TOKEN_LEFT_PARENTHESIS,
    TOKEN_RIGHT_PARENTHESIS,
    TOKEN_OPERATOR,
    TOKEN_PARAM
};

struct token_lexer_to_ast
{
    enum token_lexer_to_ast_type type;
    char *value;
};

struct compound_list_break
{
    struct ast *root;
};

struct tree* parse(struct queue *queue);