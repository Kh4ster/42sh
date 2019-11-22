#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include "../execution_handling/redirector.h"
#include "redirect.h"
#include "../parser/parser.h"
#include "../execution_handling/command_execution.h"
#include "../execution_handling/command_container.h"
#include "../parser/ast/ast.h"

static int redirect_stdin(struct redirection *redirection)
{
    int fd_saved = dup(0);

    if (fd_saved == -1)
    {
        warn("could not save stdin");
        return 1;
    }

    int filedes_file = open(redirection->file, O_RDONLY);

    if (dup2(filedes_file, 0) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_command = execute_ast(redirection->to_redirect);

    dup2(fd_saved, 0);
    close(fd_saved);
    close(filedes_file);
    return return_command;
}

static int redirect_stdout(struct redirection *redirection)
{
    int fd_saved = dup(redirection->fd);

    int filedes_file = open(redirection->file, O_WRONLY | O_CREAT | O_TRUNC,
                                                                    00666);
    if (filedes_file == -1)
        err(1, "could not open file");

    if (dup2(filedes_file, redirection->fd) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    if (dup2(fd_saved, redirection->fd) == -1)
        close(redirection->fd);

    close(filedes_file); //close file fd
    close(fd_saved);
    return return_commande;
}


static int redirect_stdout_append(struct redirection *redirection)
{
    int fd_saved = dup(redirection->fd);

    int filedes_file = open(redirection->file, O_WRONLY | O_APPEND | O_CREAT,
                                                                        00666);
    if (filedes_file == -1)
        err(1, "could not open file");

    if (dup2(filedes_file, redirection->fd) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    if (dup2(fd_saved, redirection->fd) == -1)
        close(redirection->fd);

    close(filedes_file); //close file fd
    close(fd_saved);
    return return_commande;
}


static int redirect_stdout_fd(struct redirection *redirection)
{
    int fd_to_redirect = atoi(redirection->file);
    int fd_saved = dup(redirection->fd);

    if (dup2(fd_to_redirect, redirection->fd) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_commande = 0;

    if (redirection->to_redirect != NULL)
        return_commande = execute_ast(redirection->to_redirect);

    if (dup2(fd_saved, redirection->fd) == -1)
        close(redirection->fd);

    close(fd_saved);
    return return_commande;
}


static int redirect_stdin_read_write(struct redirection *redirection)
{
    int fd_saved = dup(0);

    if (fd_saved == -1)
    {
        warn("could not save stdin");
        return 1;
    }

    int filedes_file = open(redirection->file, O_RDWR | O_CREAT, 00666);

    if (dup2(filedes_file, 0) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_command = execute_ast(redirection->to_redirect);

    dup2(fd_saved, 0);
    close(fd_saved);
    close(filedes_file); //close file fd
    return return_command;
}


static int redirect_dup_fd(struct redirection *redirection)
{
    int fd_where_redirect = atoi(redirection->file);

    int fd_saved = dup(redirection->fd);

    if (dup2(fd_where_redirect, redirection->fd) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_command = execute_ast(redirection->to_redirect);

    if (dup2(fd_saved, redirection->fd) == -1)
        close(redirection->fd);

    close(fd_saved);
    return return_command;
}


static int handle_heredoc(struct redirection *redirection)
{
    char *delimiter = redirection->file;
    FILE *temp = tmpfile();

    char *current_line = readline("> ");

    while (current_line &&
            strncmp(delimiter, current_line, strlen(delimiter)) != 0)
    {
        fputs(current_line, temp);
        fputc('\n', temp);
        free(current_line);
        current_line = readline("> ");
    }
    free(current_line);
    rewind(temp);

    int fd_saved = dup(0);

    if (fd_saved == -1)
    {
        warn("could not save stdin");
        return 1;
    }

    if (dup2(temp->_fileno, 0) == -1)
    {
        warn("could not redirect fd %d", redirection->fd);
        return 1;
    }

    int return_command = execute_ast(redirection->to_redirect);

    dup2(fd_saved, 0);
    fclose(temp);
    close(fd_saved);
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
        case TOKEN_DUP_FD:
            return redirect_dup_fd(redirect);
            break;
        case TOKEN_HEREDOC:
            return handle_heredoc(redirect);
            break;
        default:
            return 0;
    }

    return 0;
}
