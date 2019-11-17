/** @file
* @brief API to make redirection between commands and files
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
** @brief Copy stdin/out/err to 10/11/12
** @brief Allows us to then modify the stds of the bins we want to execute
** @return -1 in case of error
*/
int save_stds(void);
