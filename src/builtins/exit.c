#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <err.h>

#include "../builtins/history.h"
#include "../input_output/get_next_line.h"

int exit_builtin(char **argv)
{
    assert(argv && (strcmp(argv[0], "exit") == 0));

    if (is_interactive())
        fprintf(stderr, "exit\n");

    if (argv[1] == NULL)
    {
        write_history_file();
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
    {
        write_history_file();
        errx(2, "numeric argument required");
    }

    else
    {
        write_history_file();
        exit(return_value);
    }
}
