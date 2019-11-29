/** @file
* @brief builtin echo
* @author Coder : pierrick.made
* @author Tester :
* @author Reviewer :
*/

#pragma once

/**
* @brief replicating the behavior of echo builtin (cf. man bash-builtins)
* @param args array of arguments given to echo builtins
* @return success : 0 , fail : same behavior as echo builtin
*/
int echo(char **args);