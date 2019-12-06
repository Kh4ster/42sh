/** @file
* @brief handle command substitution
* @author Coder : nicolas.blin
* @author Tester : zakaria.ben-allal
* @author Reviewer :
*/

#pragma once

#include <stdbool.h>

/**
* @brief hat function used to expand $() and `
* @param to_expand line to expand
* @param to_stop char to stop on ) or `
* @param nb_to_skip skip 1 or to char if $( or `
* @return result of the command expansion
*/
char *hat_of_expand_cmd(char **to_expand,
                        char to_stop,
                        int nb_to_skip);

/**
 * @brief scan that only expand in case the inner_var doesn't exist
 * @param line the line to scan
 * @param is_quote if it is quote
 * @param was_quote if it was quote
 * @param inner_var var inside the $()
 * @return the result of all the expansion
*/
char *custom_scan(char *line,
                bool is_quote,
                int *was_quote,
                struct hash_map *inner_var);

/**
* @brief function used to expand $() and `
* @param to_expand line to expand
* @param to_stop char to stop on ) or `
* @param nb_to_skip skip 1 or to char if $( or `
* @param inner_var var inside the $()
* @param is_first to know if it's first call or resursiv call
* @return result of the command expansion
*/
char *expand_cmd(char *to_expand,
                char to_stop,
                int nb_to_skip,
                struct hash_map *inner_var,
                bool is_first);