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

void handle_escape_aux(char *c)
{
    if (*c == 'a')
        fprintf(stdout, "\a");
    else if (*c == 'b')
        fprintf(stdout, "\b");
    else if (*c == 'e')
        fprintf(stdout, "%c", 27);
    else if (*c == 'f')
        fprintf(stdout, "\f");
    else if (*c == 'n')
        fprintf(stdout, "\n");
    else if (*c == 'r')
        fprintf(stdout, "\r");
    else if (*c == 't')
        fprintf(stdout, "\t");
    else if (*c == 'v')
        fprintf(stdout, "\v");
    else if (*c == 'x' || *c == '0')
        fprintf(stdout, "TODO"); // octal and hexa to do
    else
        fprintf(stdout, "%c", *c);
}

int print_with_backslash_escapes(char *arg)
{
    for (char *c = arg; *c != '\0'; c++)
    {
        if (*c == '\\' && *(c + 1) != '\0')
        {
            c++;
            if (*c == '\\')
                fprintf(stdout, "\\");
            else if (*c == 'c')
            {
                return 1;
                break;
            }
            else
                handle_escape_aux(c);
        }

        else
            fprintf(stdout, "%c", *c);
    }
    return 0;
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
        {
            if (print_with_backslash_escapes(args[index]) == 1)
                return 0; // \c was called (no more output must be printed)
        }
        else
            fprintf(stdout, "%s", args[index]);

        index++;

        // print space if next arg is not NULL
        fprintf(stdout, "%s", args[index] != NULL ? " " : "");
    }

    // print trailing newline if option n not set
    fprintf(stdout, "%s", opt_n ? "" : "\n");

    return 0;
}