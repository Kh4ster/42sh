#pragma once

#include <stdbool.h>

struct boot_params
{
    bool option_n;
    bool option_a;
    bool option_o;
    bool option_c;
    char *command_option_c;
};