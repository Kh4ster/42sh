/*
** Coder : nicolas.blin
** Tester : nicolas.blin
** Reviewer :
** Integrator :
**
** wrapper on write to be sure eveything is written
*/

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "write.h"

/*
** write to fd_out
** syscall trouble handled
** return -1 in case of error
*/
int xwrite(char *buffer, int fd_out)
{
    assert(buffer != NULL);
    assert(fd_out >= 0);

    ssize_t to_write = strlen(buffer) + 1; // + 1 for \0
    ssize_t written = 0;
    while (written < to_write)
    {
        ssize_t rc = write(fd_out, buffer + written, to_write - written);
        if (rc == -1)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        written += rc;
    }
    return 0;
}