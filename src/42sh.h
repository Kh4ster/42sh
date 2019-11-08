#pragma once

#include <stdbool.h>

#include "parameters_handling/options.h"

struct shell_environment
{
    char *pwd;
    struct boot_params options;
};