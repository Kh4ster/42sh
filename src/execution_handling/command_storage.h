/** @file
* @brief stores command line in command_container
* @author Coder : zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
* @brief counts the number of words in the command line
* @param char* (command line)
* @return number of words
*/
int word_count(char *s);

/**
* @brief stores command line in command_container
* @param char* (command line)
* @return filled command_container structure
*/
struct command_container *cmd_store(char *line);
