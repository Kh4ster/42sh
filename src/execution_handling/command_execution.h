/** @file
* @brief executes commands stored in command_container structure
* @author Coder : zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
* @brief Handle parameters given to 42sh binary
* @param command_container structure
* @return 0 if success
* @return 127 if cmd not found
* @return 126 if cmd not executable
* @return 2 if cmd argument not valid
* @return -1 if execvp fails
*/
int exec_cmd(struct command_container *cmd);