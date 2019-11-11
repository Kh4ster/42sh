/** @file
* @brief executes commands stored in command_container
* @author Coder : zakaria.ben-allal
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
* @brief Handle parameters given to 42sh binary
* @param struct command_container
* @param command_container struct that will hold the commands
* @return
*/
int exec_cmd(struct command_container *cmd);
