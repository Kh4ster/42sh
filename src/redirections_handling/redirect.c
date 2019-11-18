#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

#include "../execution_handling/redirector.h"
#include "redirect.h"
#include "../parser/parser.h"
#include "../execution_handling/command_execution.h"
#include "../execution_handling/command_container.h"
#include "../parser/ast/ast.h"

static void restore_stds(void)
{
    dup2(10, 0);
    dup2(11, 1);
    dup2(12, 2);
}

static int redirect_stdin(struct redirection *redirection)
{
    if (save_stds() == -1)
        errx(1, "could not save stdin");

    int filedes_file = open(redirection->file, O_RDONLY);
    dup2(filedes_file, 0);
    int return_command = execute_ast(redirection->to_redirect);
    restore_stds();
    return return_command;
}

static int redirect_stdout(struct redirection *redirection)
{
     if (save_stds() == -1)
         errx(1, "could not save stdout");

    int filedes_file = open(redirection->file, O_WRONLY | O_CREAT | O_TRUNC,
                                                                    00666);
    if (filedes_file == -1)
        err(1, "could not open file");
    dup2(filedes_file, redirection->fd);
    int return_command = 0;
    if (redirection->to_redirect != NULL)
        return_command = execute_ast(redirection->to_redirect);
    restore_stds();
    return return_command;
}


static int redirect_stdout_append(struct redirection *redirection)
{
     if (save_stds() == -1)
         errx(1, "could not save stdout");

    int filedes_file = open(redirection->file, O_WRONLY | O_APPEND | O_CREAT,
                                                                        00666);
    if (filedes_file == -1)
        err(1, "could not open file");
    dup2(filedes_file, redirection->fd);
    int return_command = 0;
    if (redirection->to_redirect != NULL)
        return_command = execute_ast(redirection->to_redirect);
    restore_stds();
    return return_command;
}

extern int redirections_handling(struct instruction *redirection)
{
    struct redirection *redirect = redirection->data;
    switch(redirection->type)
    {
        case TOKEN_REDIRECT_LEFT:
            return redirect_stdout(redirect);
            break;
        case TOKEN_REDIRECT_APPEND_LEFT:
            return redirect_stdout_append(redirect);
            break;
        case TOKEN_REDIRECT_RIGHT:
            return redirect_stdin(redirect);
            break;
        default:
            return 1;
    }
    return 1;
}
