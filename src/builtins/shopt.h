/** @file
* @brief SHOPT builtin
* @author Coder : nicolas.blin
* @author Tester : nicolas.blin
* @author Reviewer : pierrick.made & nicolas.blin
*/

#pragma once

/**
* @brief builtin shopt following man bash
* @param options, arguments given to shopt
* @return good: 0, bad_option: 2, bad_opt_arg_name: 1
*/
int shopt(char *options[]);