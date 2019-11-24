/** @file
* @author Coder : nicolas.blin
*/

#pragma once

#include <stdbool.h>

/** @struct boot_params
* @brief To know if an option is set or not
*/
struct boot_params
{
    bool option_n; /**< @brief option norc */
    bool option_a; /**< @brief option ast-print */
    bool option_c; /**< @brief option command */
    char *command_option_c; /**< @brief command given with option c */
    bool option_dot_glob; /**< @brief option dot glob */
    bool option_expand_aliases; /**< @brief option expand alias */
    bool option_extglob; /**< @brief option extglob */
    bool option_nocaseglob; /**< @brief option no case glob */
    bool option_nullglob; /**< @brief option null glob */
    bool option_sourcepath; /**< @brief option source path */
    bool option_xpg_echo; /**< @brief option xpg echo */
    bool option_noclobber;
};
