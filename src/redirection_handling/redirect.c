#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../execution_handling/redirector.h"
#include "redirect_left.h"
#include "../parser/parser.h"
#include "../execution_handling/command_execution.h"
#include "../execution_handling/command_container.h"


static int redirect_stdin(char *file)
{
    if (save_stds() == -1)
        errx(1, "could not save stdin");

    int filedes_file = (file, O_RDONLY);
    dup2(filedes_file, 0);
    int return_commend = exec_cmd(redirection->command);
    dup2(11, 0);
    return return_command;
}


static int redirect_stdout(struct redirection *redirection)
{
     if (save_stds() == -1)
         errx(1, "could not save stdout");

    if (!redirection->redirection)
        redirection->redirection = 1;

    int filedes_file = (file, O_WRONLY);
    dup2(filedes_file, redirection->redirection);
    int return_commande = exec_cmd(redirection->command);
    dup2(11, 1);
    return return_commande;
}


static int redirect_stdout_append(struct redirection *redirection)
{
     if (save_stds() == -1)
         errx(1, "could not save stdout");

    if (!redirection->redirection)
        redirection->redirection = 1;

    int filedes_file = (file, O_APPEND);
    dup2(filedes_file, redirection->redirection);
    int return_commande = exec_cmd(redirection->command);
    dup2(11, 1);
    return return_commande;
}


extern int redirection_handling(struct redirection *redirect)
{
    switch(redirect->redirection_type)
    {
        case TOKEN_REDIRECT_LEFT:
            return redict_stdout(redirect);
            break;
        case TOKEN_REDIRECT_APPEND_LEFT:
            return redirect_stdout(redirect);
            break;
        case TOKEN_REDIRECT_RIGHT:
            return redirect_stdout(redirect);
            break;
        default:
            return 1;
    }
    return 1;
}
