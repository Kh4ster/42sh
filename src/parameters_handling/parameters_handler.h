#pragma once

#include "options.h"

/**
* @brief Handle parameters given to 42sh binary
* @param options struct that will hold the data
* @param argc same argc as main
* @param argv same argv as main
* @return -1 : fail, 1 : success
*/
int handle_parameters(struct boot_params *options, int argc, char *argv[]);