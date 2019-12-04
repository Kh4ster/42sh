/** @file
* @brief handle command expansion
* @author Coder : nicolas.blin
* @author Tester : zakaria.ben-allal
* @author Reviewer :
*/

#pragma once

#include "../parser/parser.h"

int handle_expand_command(struct instruction *command_i);
char *expand(char **to_expand, bool is_quote, int *was_quote);
char *scan_for_expand(char *line, bool is_quote, int *was_quote);