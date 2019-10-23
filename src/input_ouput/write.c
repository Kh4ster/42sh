/*
** Coder : nicolas.blin
** Tester : 
** Reviewer : 
** Integrator : 
**
** wrapper on write to be sure eveything is written
** NEED TO HANDLE SYSCALL INTERPUT
*/

#include <string.h>
#include <unistd.h>

#include "write.h"

int xwrite(char *buffer, int fd_out)
{
    assert(buffer != NULL);
    assert(fd_out >= 0);
    ssize_t to_write = strlen(buffer) + 1; // + 1 for \0
    ssize_t written = 0;
    do {
        written += write(fd_out, buffer, to_write);
        if (written < to_write)
            continue;
        else
            break;
    } while (1);
}