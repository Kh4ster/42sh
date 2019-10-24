/*
** Coder : nicolas.blin
** Tester : nicolas.blin
** Reviewer : 
** Integrator : 
**
** wrapper on read to be sure eveything is written
*/

#define CHUNK_READ_SIZE 64

#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "read.h"
#include "../data_structures/data_string.h"

/*
** read from fd_in
** syscall trouble handled
** return -1 in case of error
*/
int xread(struct string *string, int fd_in)
{
    assert(string != NULL);
    assert(fd_in >= 0);

    ssize_t res = 0;
    char buffer[64];
    do
    {
        res = read(fd_in, buffer, sizeof(buffer) - 1); //for \0
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (res != 0)
        {
            buffer[res] = 0;
            append_n_string(string, buffer, res + 1);
        }
    } while (res != 0);
    return 0;
}