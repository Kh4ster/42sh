#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "command_container.h"
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
            errx(127, "127");
        else if (errno == EACCES)
            errx(126, "126");
        else
            errx(-1, "-1");
    }
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
}

/**might replace criterion unitary testing

int main()
{
    struct command_container *cmd = command_init(1, "toto", "toto");
    int exec = exec_cmd(cmd);
    command_destroy(&cmd);
    return exec;
}

*/
