#pragma once

#include <stdbool.h>

#include "parameters_handling/options.h"

/** @struct name_struct
* @brief Holds information about the shell execution environnement
*/
struct shell_environment
{
    char *pwd; /**< @brief shell current position */
    struct boot_params options; /**< @brief parameters given to 42sh */
};