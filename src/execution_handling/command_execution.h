/** @file
* @brief executes commands stored in command_container structure
* @author Coder : zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer : pierrick.made & nicolas.blin
*/

#pragma once

#include <stdbool.h>

#include "command_container.h"
#include "../parser/parser.h"
#include "../data_structures/hash_map.h"

/**
* @brief Handle parameters given to 42sh binary
* @param command_container structure
* @return 0 if success
* @return 127 if cmd not found
* @return 126 if cmd not executable
* @return 2 if cmd argument not valid
* @return -1 if execvp fails
*/
int exec_cmd(struct instruction *cmd_container);

/**
 * @brief execute the command with our 42sh binary
 * @param command the command to execute
 * @param inner_var hash_map used to know if var was define in $()
 * @param is_first to know if it's first call or resursiv call
 * @return the result of the command execution
*/
char *get_result_from_42sh(char *command,
                            struct hash_map *inner_var,
                            bool is_first);