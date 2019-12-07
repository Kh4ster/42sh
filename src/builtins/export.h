/** @file
* @brief Export builtin
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
//void simple_export();
int export(char **argv);