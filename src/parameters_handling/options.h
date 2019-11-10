#pragma once

#include <stdbool.h>

/** @struct boot_params
* @brief To know if an option is set or not
*/
struct boot_params
{
    bool option_n; /**< @brief option n */
    bool option_a; /**< @brief option a */
    bool option_o; /**< @brief option o */
    bool option_c; /**< @brief option c */
    char *command_option_c; /**< @brief command given with option c */
};