#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <err.h>

#include "../input_output/get_next_line.h"

int exit_builtin(char *argv[])
{
    assert(argv && (strcmp(argv[0], "exit") == 0));

    if (is_interactive())
        printf("exit\n");

    if (argv[1] == NULL)
    {
        exit(g_env.last_return_value);
    }

    if (argv[0] != NULL && argv[1] != NULL)
    {
        char *endptr;
        long int return_value = strtol(argv[1], &endptr, 10);

        if (argv[2] != NULL && endptr == NULL)
        {
            if (g_env.last_return_value == 0)
                errx(1, "too many arguments");
            else
                errx(g_env.last_return_value, "too many arguments");
        }

        if (endptr != NULL)
            errx(2, "numeric argument required\n");

        else
            exit((return_value + g_env.last_return_value) % 256);
    }
}
