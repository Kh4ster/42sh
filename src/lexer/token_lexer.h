/** @file
* @brief Data_structure of token used and returned by lexer
* @author Coder : pierrick.made
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

enum token_lexer_type
{
    TOKEN_END_OF_INSTRUCTION,
    TOKEN_END_OF_LINE,
    TOKEN_OTHER
};

struct token_lexer
{
    char *data;
    enum token_lexer_type type;
};

struct queue *lexer(char *line);