/** @file
* @brief API to make pipe between commands and command
* @author Coder : nicolas.blin
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../execution_handling/command_container.h"

struct tube
{
    int read;
    int write;
};

/**
* @brief creates a pipe a fill it using the first command
* @param command command to execute
* @return tube struct : success ; NULL : fail
*/
struct tube* fill_pipe_from_stdin(struct command_container *command);

/**
* @brief free the tube closing the fd and calling free on it
* @param tube this is the tree to destroy
*/
void tube_free(struct tube *tube);

/**
* @brief read from the pipe and fill it again by executing the command
* @param command command to execute
* @param tube the pipe
* @return 0 : success ; !0 : fail
*/
int read_and_fill_pipe(struct command_container *command,
                                struct tube *tube);

/**
* @brief the last command read from the pipe and write in stdout
* @param command command to execute
* @return 0 : success ; !0 : fail
*/
int write_pipe_to_stdout(struct tube *tube, struct command_container *command);

int save_stds(void);

int restore_stds(void);