/** @file
* @brief builtin history with -c (clear) and (-r) restore options
* @author Coder : pierrick.made
* @author Tester : pierrick.made
* @author Reviewer : nicolas.blin & pierrick.made
*/

#pragma once

/**
* @brief give the path to the history file of 42sh (default: ~/.42sh_history)
* @return success : char * containing the path to history file, fail : NULL
*/
char *get_history_file_path(void);

/**
* @brief replicating the behavior of history builtin (cf. man bash-builtins)
* @param args array of arguments given to history builtins
* @return success : 0 , fail : same behavior as history builtin
*/
int history(char **args);