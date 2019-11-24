/** @file
* @brief source file to handle the redirections from stdout to a file
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../parser/parser.h"

/**
** @brief: function that takes a redirection and acts depending on its type:
** @brief if it is a left redirection, set stdout to the specified file. If if
** @brief is a >>, set stdout to the specified file in append mode. if it is
** @brief a <, set stdin to the specified file.
** @param redirection: This is the struct redirection to handle
** @return 0 if no problem happend, error code otherwise
*/

extern int redirections_handling(struct instruction *redirection, int execute);
