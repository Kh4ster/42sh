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

int save_stds(void)
{
    int errors[3];
    errors[0] = dup2(0, 10);
    errors[1] = dup2(1, 11);
    errors[2] = dup2(2, 12);
    for (short i = 0; i < 3; i++)
    {
        if (errors[i] == -1)
            return -1;
    }
    return 0;
}
