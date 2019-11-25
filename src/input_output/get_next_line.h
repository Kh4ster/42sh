/** @file
* @brief handling prompt script
* @author Coder : nicolas.blin & pierrick.made
* @author Tester : nicolas.blin
* @author Reviewer : pierrick.made & nicolas.blin
*/

#pragma once

#include "../parameters_handling/parameters_handler.h"
#include "../data_structures/hash_map.h"

extern struct shell_environment g_env;

/**
* @struct shell_environment
* @brief Holds information about the shell execution environnement
*/
struct shell_environment
{
    char *pwd; /**< @brief shell current position */
    struct boot_params options; /**< @brief parameters given to 42sh */
    char *prompt; /**< @brief ps1 or ps2 prompt */
    int is_parsing_ressource; /**< @brief check if valid */
    struct hash_map *functions; /**< @brief hashmap containing functions */
    struct hash_map *builtins; /**< @brief hashmap containing builtins */
    char *current_line; /**< @brief last line took by readline */
    int noclobber_set; /**< @brief handle noclobber variable */
    int last_return_value; /** @brief last returned value */
};

/**
* @brief check if interactive mode is on
* @return isatty(tty)
*/
int is_interactive(void);

/**
* @brief returns a new line from the received prompt string
* @param prompt string
* @return new line string
*/
char *get_next_line(const char *prompt);
