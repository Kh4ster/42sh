#include <err.h>
#include <stdio.h>

#include "echo.h"

void handle_options(char **args, int *index, int *opt_e, int *opt_n)
{
    // used to break loop in case of bad options
    int bad_opt = 0;

    while (args[*index] != NULL && args[*index][0] == '-' && !bad_opt)
    {
        // save old options in case of bad options (to restore them)
        int old_opt_n = *opt_n;
        int old_opt_e = *opt_e;

        // check if it is a single dash
        if (args[*index][1] == '\0')
            bad_opt = 1;

        for (char *c = args[*index] + 1; *c != '\0' && !bad_opt; c++)
        {
            if (*c == 'n')
                *opt_n = 1;
            else if (*c == 'e')
                *opt_e = 1;
            else if (*c == 'E')
                *opt_e = 0;
            else
            {
                bad_opt = 1;
                *opt_e = old_opt_e;
                *opt_n = old_opt_n;
            }
        }
        if (!bad_opt)
            (*index)++;
    }
}

int echo(char **args)
{
    // options e, n and E of echo
    int opt_n = 0;
    int opt_e = 0; // E: 0, e: 1

    // set index of current arg
    int index = 1;

    // Set options and put the index at the next arg to print
    handle_options(args, &index, &opt_e, &opt_n);

    // print all args depending on set options
    while (args[index] != NULL)
    {
        if (opt_e)
            printf("%s", args[index]); //print_opt_e(args);
        else
            printf("%s", args[index]);

        index++;

        // print space if next arg is not NULL
        printf("%s", args[index] != NULL ? " " : "");
    }

    // print trailing newline if option n not set
    printf("%s", opt_n ? "" : "\n");

    return 0;
}

int main(int argc, char **argv)
{
    echo(argv);
    (void)argc;
}