/** @file
* @brief executes commands stored in command_container
* @author Coder : zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer :
* @author Integrator :
*/

#pragma once

/**
* @brief Handle parameters given to 42sh binary
* @param struct command_container
* @param command_container struct that will hold the commands
* @return 0 if success
* @return 127 if cmd not found
* @return 126 if cmd not executable
* @return 2 if cmd argument not valid
*/

int exec_cmd(struct command_container *cmd);
