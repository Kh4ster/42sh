/** @file
* @brief builtin cd
* @author Coder : pierrick.made
* @author Tester :
* @author Reviewer :
*/

#pragma once

/**
* @brief replicating the behavior of cd builtin (cf. man bash-builtins)
* @param args array of arguments given to cd builtins
* @return success : 0 , fail : same behavior as cd builtin
*/
int cd(char **args);