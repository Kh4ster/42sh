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
#include "../path_expention/path_exepension.h"
#include "../memory/memory.h"
#include "../data_structures/array_list.h"
#include "../input_output/read.h"
#include "../data_structures/data_string.h"
#include "../input_output/get_next_line.h"
#include "../command_substitution/command_substitution.h"

int exec_cmd(struct instruction *cmd_container)
{
    struct command_container *cmd = cmd_container->data;

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

static void trim_return_line(char *str)
{
    char *beg = str;
    str += strlen(str) - 1;
    //- 1 cause we can remove the char beg but not dereference the char before
    while (str != beg - 1 && *str == '\n')
    {
        *str = '\0';
        str--;
    }
}

char *get_result_from_42sh(char *command, struct hash_map *inner_var)
{
    int was_quoted = 0;
    command = custom_scan(command, false, &was_quoted, inner_var);

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
        execl(g_env.path_to_binary, "42sh", "-c", command, NULL);
        errx(-1, "execvp has failed"); //-1 ? execl ?
    }
    close(1); //close write side of pipe in stdout, if not xread loops forever
    int wstatus;
    waitpid(pid, &wstatus, 0); //return code ?

    struct string *str = string_init(); //retrieve data from pipe
    if (xread(str, tube[0]) == -1)
        errx(-1, "Bad read"); //-1 ?

    trim_return_line(str->content);

    dup2(save_stdout, 1); //put back stdout
    close(tube[0]);     //close read side
    close(save_stdout);
    free(command);

    if (was_quoted)
    {
        char *quoted_result = xcalloc(str->index + 3, sizeof(char));
        if (was_quoted == 2)
            strcat(quoted_result, "\"");
        else //single quote
            strcat(quoted_result, "\"");
        strcat(quoted_result, str->content);
        if (was_quoted == 2)
            strcat(quoted_result, "\"");
        else //single quote
            strcat(quoted_result, "\"");
        string_free(&str);
        return quoted_result;
    }

    return string_get_content(&str);
}
