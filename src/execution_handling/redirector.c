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
#include <stdlib.h>

#include "redirector.h"
#include "command_container.h"

extern void free_one_fd(int fd)
{
    close(fd);
}

extern int save_one_fd(int fd_save, int fd)
{
    int error = dup2(fd, fd_save);

    if (error == -1)
        return -1;

    return 0;
}