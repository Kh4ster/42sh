/** @file
* @brief Command_expansion
* @author Coder : all
* @author Tester : all
* @author Reviewer : all
*/

#pragma once

int handle_expand_command(struct instruction *command_i);
char *expand(char **to_expand, bool is_quote, int *was_quote);
char *scan_for_expand(char *line, bool is_quote, int *was_quote);