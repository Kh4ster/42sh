/** @file
* @brief utils for parser
* @author Coder : nicolas.blin & cloe.lacombe
* @author Tester : pierrick.made & zakaria.ben-allal
* @author Reviewer : pierrick.made
*/

#pragma once
#include "../lexer/token_lexer.h"

bool is_function(struct queue *lexer);

enum token_parser_type is_redirection(struct queue *lexer);

bool is_shell_command(struct queue *lexer);

bool next_is_assignement(struct queue *lexer);

bool next_is_assignement(struct queue *lexer);

bool next_is_other(struct queue *lexer);

bool next_is_number(struct queue *lexer);

bool next_is_control_structure(struct queue *lexer);

int next_is_end_of_case_item(struct queue *lexer);

bool next_is_end_of_instruction(struct queue *lexer);

bool next_is_eof(struct queue *lexer);

bool next_is(struct queue *lexer, const char *to_match);

bool next_next_is(struct queue *lexer, const char *to_match);

enum token_parser_type token_is_redirection (struct token_lexer *token);