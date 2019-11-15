/** @file
* @brief source file to handle the redirections from stdout to a file like this : "command" > file
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../parser/parser.h"

extern int redirections_handling(struct instruction *redirection);