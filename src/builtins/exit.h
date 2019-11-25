/** @file
* @brief EXIT builtin
* @author Coder : zakaria.ben-allal
* @author Tester :
* @author Reviewer :
*/

#pragma once

/**
* @brief exit builtin
* @param char**
* @return no arguments: 0 or error value, too much arguments: 1
* @return wrong argument: 2
*/
int exit_builtin(char *argv[]);
