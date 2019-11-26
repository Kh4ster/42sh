/** @file
* @brief EXIT builtin
* @author Coder : zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer : nicolas.blin & pierrick.made
*/

#pragma once

/**
* @brief exit builtin
* @param argv arguments given to the builtin exit
* @return no arguments: 0 or error value, too much arguments: 1
* @return wrong argument: 2
*/
int exit_builtin(char *argv[]);
