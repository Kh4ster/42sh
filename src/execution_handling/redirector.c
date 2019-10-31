/*
** Coder : nicolas.blin
** Tester :
** Reviewer :
** Integrator :
**
** API to make redirection between commands and files
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/memfd.h>
#include <err.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>

#include "redirector.h"
#include "command_container.h"

static jmp_buf g_jmp_buffer;

#define TRY switch (setjmp(g_jmp_buffer))\
            {\
            case 0:
#define CATCH(x) break;\
                case x:
#define ETRY }
#define THROW(x) longjmp(g_jmp_buffer, x)

#define PIPE_EXCEPTION (1)
#define DUP2_EXCEPTION (2)
#define EXEC_EXCEPTION (3)
#define FORK_EXCEPTION (4)
#define EXCEPTION (5)

static void execute_command(struct command_container *command)
{
    assert(command != NULL);
    assert(command->command != NULL);

    int pid = fork();
    if (pid == -1)
        THROW(EXCEPTION);
    if (pid == 0)
    {
        execv(command->command, command->params);
        THROW(EXCEPTION);
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) != 0)
            THROW(EXCEPTION);
    }
}

/*
* Copy stdin/out/err to 10/11/12
* Allows us to then modify the stds of the bins we want to execute
* And then get back the true stds
*/
void save_stds(void)
{
    int errors[3];
    errors[0] = dup2(0, 10);
    errors[1] = dup2(1, 11);
    errors[2] = dup2(2, 12);
    for (size_t i = 0; i < 3; ++i)
        if (errors[i] == -1)
            THROW(EXCEPTION);
}

static int execute_first(int fd_in, struct command_container *command)
{
    assert(fd_in >= 0);

    int fd[2];
    if (pipe(fd) == -1)
        THROW(EXCEPTION);
    if (dup2(fd[1], 1) == -1) //Replace stdout with pipe
        THROW(EXCEPTION);
    if (dup2(fd_in, 0) == -1) //Replace stdin with argument
        THROW(EXCEPTION);
    TRY
    {
        execute_command(command);
    }
    CATCH(EXEC_EXCEPTION)
    {
        close(fd[1]);
        THROW(EXEC_EXCEPTION);
    }
    ETRY
    close(fd[1]);
    return fd[0];
}

static void execute_second(int fd_pipe_stdout,
                            struct command_container *command,
                            int fd_out)
{
    assert(fd_pipe_stdout >= 0 && fd_out >= 0);

    if (dup2(fd_pipe_stdout, 0) == -1) //Replace stdin with pipe
        THROW(EXCEPTION);
    if (dup2(fd_out, 1) == -1) //Replace stdout with param
        THROW(EXCEPTION);
    execute_command(command);
    close(fd_pipe_stdout);
}

int redirect_command_to_command(int fd_in,
                                struct command_container *command_in,
                                struct command_container *command_out,
                                int fd_out)
{
    int fd_pipe_stdout = -1;
    TRY
    {
        fd_pipe_stdout = execute_first(fd_in, command_in);
        execute_second(fd_pipe_stdout, command_out, fd_out);
    }
    CATCH(EXCEPTION)
    {
        if (fd_pipe_stdout != -1)
            close(fd_pipe_stdout);
        return -1;
    }
    ETRY
    return 1;
}