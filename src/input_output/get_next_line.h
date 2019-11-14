/** @file
* @brief 
* @author Coder :
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../parameters_handling/parameters_handler.h"

extern struct shell_environment g_env;

/** @struct shell_environment
* @brief Holds information about the shell execution environnement
*/
struct shell_environment
{
    char *pwd; /**< @brief shell current position */
    struct boot_params options; /**< @brief parameters given to 42sh */
    char *prompt;
};

char *get_next_line(const char *prompt);