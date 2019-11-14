/** @file
* @brief Handle entry parameters given to 42sh binary
* @author Coder : nicolas.blin & zakaria.ben-allal
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "options.h"

/**
* @brief Handle parameters given to 42sh binary
* @param options struct that will hold the data
* @param argc same argc as main
* @param argv same argv as main
* @param fd : file descriptor of the file to be opened
* @return -1 : fail, 1: success
*/
int handle_parameters(struct boot_params *options, int argc, char *argv[]
                      , int fd);

/**
* @brief open file given in parameters and make it replace stdin
* @param fd : file descriptor of the file to be opened
* @param argv same argv as main
* @return -1 : fail, 0: success
*/
int handle_file(int fd, char *argv[]);
