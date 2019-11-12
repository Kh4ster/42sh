/** @file
* @brief stores command line in command_container
* @author Coder : zakaria.ben-allal
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
* @brief stores command line in command_container
* @param char* (command line)
* @return filled command_container structure
*/
struct command_container *cmd_store(char *line);
