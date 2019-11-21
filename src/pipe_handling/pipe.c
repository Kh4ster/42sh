#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "../execution_handling/command_container.h"
#include "../execution_handling/command_execution.h"
#include "pipe.h"
#include "../memory/memory.h"

/*
** Copy stdin/out/err to 20/21/22
** Allows us to then modify the stds of the bins we want to execute
** And then get back the true stds
** return -1 in case of error
*/
int save_stds(void)
{
    int errors[3];
    errors[0] = dup2(0, 20);
    errors[1] = dup2(1, 21);
    errors[2] = dup2(2, 22);
    for (short i = 0; i < 3; i++)
        if (errors[i] == -1)
            return -1;
    return 0;
}

int restore_stds(void)
{
    int errors[3];
    errors[0] = dup2(20, 0);
    errors[1] = dup2(21, 1);
    errors[2] = dup2(22, 2);
    for (short i = 0; i < 3; i++)
        if (errors[i] == -1)
            return -1;
    close(20);
    close(21);
    close(22);
    return 0;
}

struct tube* fill_pipe_from_stdin(struct command_container *command)
{
    int fd[2];
    if (pipe(fd) == -1) //create the pipe that will be used to transmit data
        return NULL;

    struct tube *tube = xmalloc(sizeof(*tube));
    tube->read = fd[0];
    tube->write = fd[1];


    if (dup2(tube->write, 1) == -1) //Replace stdout with pipe
        return NULL;

    int return_value;
    if ((return_value = exec_cmd(command)) != 0)
    {
        close(fd[0]);
        close(fd[1]);
        free(tube);
        return NULL;
    }
    return tube;
}

int read_and_fill_pipe(struct command_container *command,
                                struct tube *tube)
{
    if (dup2(tube->read, 0) == -1) //Replace stdin with tube read
        return -1;
    if (dup2(tube->write, 1) == -1) //Replace stdout with tube write
        return -1;
 
    return exec_cmd(command);
}

void tube_free(struct tube *tube)
{
    close(tube->read);
    close(tube->write);
    free(tube);
}

int write_pipe_to_stdout(struct tube *tube, struct command_container *command)
{
    if (dup2(tube->read, 0) == -1) //Replace stdin with tube out
        return -1;
    if (dup2(21, 1) == -1) //Replace stdout with tube int
        return -1;

    return exec_cmd(command);
}