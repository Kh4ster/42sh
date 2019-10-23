/*
** Coder : nicolas.blin
** Tester : 
** Reviewer : 
** Integrator : 
**
** wrapper on read to be sure eveything is written
** USE READ IMPLEM IN CHEWIE COURSE
*/

#define CHUNK_READ_SIZE 64

#include <unistd.h>
#include <assert.h>

#include "read.h"
#include "../data_structures/data_string.h"

int xread(struct string *string, int fd_in)
{
    assert(string != NULL);
    assert(fd_in >= 0);

    ssize_t res = 0;
    char buffer[64];
    do {
        res = read(fd_in, buffer, CHUNK_READ_SIZE);
        if (res == 0 || res == -1)
            break;
        append_n_string(string, buffer, res);
    } while (1);
}