/** @file
* @brief handling prompt script
* @author Coder :
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../parameters_handling/parameters_handler.h"

extern struct shell_environment g_env;

/** @struct shell_environment
** @brief Holds information about the shell execution environnement
*/
struct shell_environment
{
    char *pwd; /**< @brief shell current position */
    struct boot_params options; /**< @brief parameters given to 42sh */
    char *prompt; /**< @brief ps1 or ps2 prompt */
    int not_first_line; /**< @brief used to know if is first call to getline */
    int is_parsing_ressource; /**< @brief check if valid */
};

/**
** @brief check if interactive mode is on
** @return isatty(tty)
*/
int is_interactive(void);

/**
** @brief returns a new line from the received prompt string
** @param prompt string
** @return new line string
*/
char *get_next_line(const char *prompt);
