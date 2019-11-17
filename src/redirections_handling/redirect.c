#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdlib.h>

#include "../execution_handling/redirector.h"
#include "redirect.h"
#include "../parser/parser.h"
#include "../execution_handling/command_execution.h"
#include "../execution_handling/command_container.h"
#include "../parser/ast/ast.h"


int fd_to_save_g = 42;

static int redirect_stdin(struct redirection *redirection)
{
    if (save_stds() == -1)
        errx(1, "could not save stdin");

    int filedes_file = open(redirection->file, O_RDONLY);
    dup2(filedes_file, 0);
    int return_command = execute_ast(redirection->to_redirect);
    dup2(11, 0);
    return return_command;
}

static int redirect_stdout(struct redirection *redirection)
{
    int fd_to_save = fd_to_save_g;
    fd_to_save_g++;

     if (save_one_fd(fd_to_save, redirection->fd) == -1)
         errx(1, "could not save stds");

    int filedes_file = open(redirection->file, O_WRONLY | O_CREAT | O_TRUNC,
                                                                    00666);
    if (filedes_file == -1)
        err(1, "could not open file");

    dup2(filedes_file, redirection->fd);
    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    dup2(fd_to_save, redirection->fd); //closes file fd
    free_one_fd(fd_to_save);
    return return_commande;
}


static int redirect_stdout_append(struct redirection *redirection)
{
    int fd_to_save = fd_to_save_g;
    fd_to_save_g++;

    if (save_one_fd(fd_to_save, redirection->fd) == -1)
         errx(1, "could not save stds");

    int filedes_file = open(redirection->file, O_WRONLY | O_APPEND | O_CREAT,
                                                                        00666);
    if (filedes_file == -1)
        err(1, "could not open file");

    dup2(filedes_file, redirection->fd);
    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    dup2(fd_to_save, redirection->fd); //close file fd
    free_one_fd(fd_to_save);
    return return_commande;
}


static int redirect_stdout_fd(struct redirection *redirection)
{
    int fd_to_save = fd_to_save_g;
    fd_to_save_g++;
    int fd_to_redirect = atoi(redirection->file);

    if (save_one_fd(fd_to_save, redirection->fd) == -1)
         errx(1, "could not save stds");

    dup2(fd_to_redirect, redirection->fd);
    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    dup2(fd_to_save, redirection->fd); //close file fd
    free_one_fd(fd_to_save);
    return return_commande;
}


static int redirect_stdin_read_write(struct redirection *redirection)
{
    if (save_stds() == -1)
        errx(1, "could not save stdin");

    int filedes_file = open(redirection->file, O_RDWR | O_CREAT, 00666);
    dup2(filedes_file, 0);
    int return_command = execute_ast(redirection->to_redirect);
    dup2(11, 0);
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
        case TOKEN_REDIRECT_LEFT_TO_FD:
            return redirect_stdout_fd(redirect);
            break;
        case TOKEN_REDIRECT_READ_WRITE:
            return redirect_stdin_read_write(redirect);
            break;
        default:
            return 1;
    }
    fd_to_save_g = 42;
    return 1;
}
