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
#include "../input_output/read.h"
#include "../data_structures/data_string.h"

int exec_cmd(struct command_container *cmd)
{
    int pid = fork();
    if (pid == -1)
        exit(EXIT_FAILURE);
    if (pid == 0)
    {
        execvp(cmd->command, cmd->params);
        if (errno == ENOENT)
        {
            errx(127, "command not found");
        }
        else if (errno == EACCES)
            errx(126, "cannot execute command");
        else
            errx(-1, "execvp has failed");
    }
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
}

char *get_result_from_42sh(char *command)
{
    int tube[2];
    if (pipe(tube) == -1)
        errx(-1, "Bad pipe"); //-1 ?

    int save_stdout = dup(1); //save stdout
    dup2(tube[1], 1);   //replace stdout
    close(tube[1]);     //close write side tube

    int pid = fork();
    if (pid == -1)
        exit(EXIT_FAILURE);
    if (pid == 0)
    {
        close(tube[0]); //close read side
        execl("./42sh", "42sh", "-c", command, NULL);
        errx(-1, "execvp has failed"); //-1 ? execl ?
    }
    close(1); //close write side of pipe in stdout, if not xread loops forever
    int wstatus;
    waitpid(pid, &wstatus, 0); //return code ?

    struct string *str = string_init(); //retrieve data from pipe
    if (xread(str, tube[0]) == -1)
        errx(-1, "Bad read"); //-1 ?

    dup2(save_stdout, 1); //put back stdout
    close(tube[0]);     //close read side
    close(save_stdout);

    return string_get_content(&str);
}