#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <err.h>

#include "../input_output/get_next_line.h"

int exit_builtin(char **argv)
{
    assert(argv && (strcmp(argv[0], "exit") == 0));

    if (is_interactive())
        printf("exit\n");

    if (argv[1] == NULL)
    {
        exit(g_env.last_return_value);
    }
    int base = 10;
    char *error;
    char return_value = strtol(argv[1], &error, base);

    if (argv[2] != NULL && *error == 0)
    {
        fprintf(stderr, "too many arguments\n");

            if (g_env.last_return_value == 0)
                return 1;
            else
                return g_env.last_return_value;
    }
    if (*error != 0)
        errx(2, "numeric argument required");

    else
        exit(return_value);
}
