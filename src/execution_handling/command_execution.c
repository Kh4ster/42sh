#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "command_execution.h"

int exec_cmd(struct command_container *cmd)
{
    int pid = fork();
    if (pid == -1)
        exit(EXIT_FAILURE);
    if (pid == 0)
    {
        execvp(cmd->command, cmd->params);
        if (errno == ENOENT)
            return 127;
        if (errno == EACCES)
            return 126;
    }
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
}

int main(int argc, char **argv)
{
    if (argc < 1)
        exit(EXIT_FAILURE);
    char *cmd_name = "./toto";
    struct command_container *cmd = malloc(sizeof(struct command_container));
    cmd->command = cmd_name;
    cmd->params = argv;
    int exec = exec_cmd(cmd);
    return exec;
}
