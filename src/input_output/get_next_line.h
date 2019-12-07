/** @file
* @brief contain all g_env and handle next line
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
    int prompt; /**< @brief ps1 or ps2 prompt */
    int is_parsing_ressource; /**< @brief check if valid */
    struct hash_map *functions; /**< @brief hashmap containing functions */
    struct hash_map *builtins; /**< @brief hashmap containing builtins */
    struct hash_map *variables; /**< @brief hashmap containing variables */
    struct hash_map *aliases; /**< @brief hashmap containing aliases */
    char *current_line; /**< @brief last line took by readline */
    int breaks; /**< @brief to deceremente in the case beak n */
    int continues; /**< @brief to deceremente in the case continues n */
    int is_in_loop; /**< @brief to know if we are in a loop */
    int noclobber_set; /**< @brief handle noclobber variable */
    char last_return_value; /**< @brief last returned value */
    char *path_to_binary; /**< @brief where the binary is executed */
    char **envvar; /**< @brief containing environment variables list*/
    char **old_envvar; /**< @brief containing old environment variables list*/
    char *old_pwd; /**< @brief to know the old pwd and handle cd - */
    int argc; /**< @brief to handle special variable $1 */
    char **argv; /**< @brief to handle special variable $1 */
};

/**
* @brief check if interactive mode is on
* @return isatty(tty)
*/
int is_interactive(void);

/**
* @brief returns a new line from the received prompt int
* @param prompt number
* @return new line string
*/
char *get_next_line(int prompt);

/**
* @brief corresponding to PS[1-2]
* @param ps_num 1 or 2 corresponding to the PS1 or PS2
*/
char *get_prompt(int ps_num);