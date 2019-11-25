#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "get_next_line.h"

int exit_builtin(char *argv[])
{
    assert(argv && (strcmp(argv[0], "exit") == 0));

    int return_value = 0;
    char *endptr;
    if (argv[0] && !argv[1])
    {
        if (is_interactive())
        {
            printf("exit\n");
            exit(return_value + g_env.last_return_value);
        }
        else
            exit(return_value + g_env.last_return_value);
    }

    if (argv[0] && argv[1] && !argv[2])
    {
        return_value = strtol(argv[1], &endptr, 10);

        if (*endptr)
        {
            errx(2, "numeric argument required\n");
        }

        if (is_interactive())
        {
            printf("exit\n");
            exit(return_value + g_env.last_return_value);
        }
        else
            exit(return_value + g_env.last_return_value);
    }

    if (argv[2])
    {
        if (g_env.last_return_value == 0)
            errx(1, "too many arguments");
        else
            errx(g_env.last_return_value, "too many arguments");
    }

    else
        return -1;
}

