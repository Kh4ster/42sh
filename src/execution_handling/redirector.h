/** @file
* @brief API to make redirection between commands and files
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "command_container.h"

/**
** @brief Copy stdin/out/err to 10/11/12
** @brief Allows us to then modify the stds of the bins we want to execute
** @return -1 in case of error
*/
#define FREE_DEFAULT_FD() free_one_fd(42)

extern void free_one_fd(int fd);

extern int save_one_fd(int fd_save, int fd);

int redirect_command_to_command(int fd_in,
                                struct command_container *command_in,
                                struct command_container *command_out,
                                int fd_out);

int redirect_command_to_stdout(int fd_in,
                                struct command_container *command_out);

int redirect_command_to_file(int fd_in,
                            struct command_container *command,
                            int fd_file);

/**
** @brief Copy stdin/out/err to 10/11/12
** @brief Allows us to then modify the stds of the bins we want to execute
** @return -1 in case of error
*/
int save_stds(void);