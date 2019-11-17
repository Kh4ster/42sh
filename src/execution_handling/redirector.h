#pragma once

#include "command_container.h"

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

int restore_stds(void);

int save_stds(void);
