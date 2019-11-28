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

#if 0
static size_t get_nb_params(char **params)
{
    int res = 0;

    for (int i = 0; params[i]; i++)
    {
        res++;
    }

    return res + 1;
}
#endif


static struct array_list *__add_glob_to_cmd(char **params, char *pattern,
                    int *i)
{
    struct path_globbing *glob = sh_glob(pattern);

    if (!glob)
        return NULL;

    struct array_list *list = array_list_init();

    for (int j = 1; j < *i; j++)
        array_list_append(list, strdup(params[j]));

    for (int j = 0; j < glob->nb_matchs; j++)
        array_list_append(list, strdup(glob->matches->content[j]));

    (*i)++;
    for (; params[*i]; (*i)++)
        array_list_append(list, strdup(params[*i]));

    destroy_path_glob(glob);
    return list;
}


static struct command_container *add_glob_to_cmd(struct command_container *cmd)
{
    for (int i = 0; cmd->params[i]; i++)
    {
        struct array_list *list = __add_glob_to_cmd(cmd->params,
                                cmd->params[i], &i);

        if (!list)
            continue;

        struct command_container *new =
                    command_create(strdup(cmd->command), list);
        free(list->content);
        free(list);
        command_destroy(&cmd);
        cmd = new;
    }
    return cmd;
}


int exec_cmd(struct instruction *cmd_container)
{
    cmd_container->data = add_glob_to_cmd(cmd_container->data);
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

    str->content[strlen(str->content) - 1] = 0; //TODO, dunno why has to remove \n

    dup2(save_stdout, 1); //put back stdout
    close(tube[0]);     //close read side
    close(save_stdout);

    return string_get_content(&str);
}
