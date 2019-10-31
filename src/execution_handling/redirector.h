#pragma once

#include "command_container.h"

int redirect_command_to_command(int fd_in,
                                struct command_container *command_in,
                                struct command_container *command_out,
                                int fd_out);

int redirect_command_to_stdout(int fd_in,
                                struct command_container *command_out);

int redirect_command_to_file(int fd_in,
                            struct command_container *command,
                            int fd_file);

void save_stds(void);