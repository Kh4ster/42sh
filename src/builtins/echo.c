#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "echo.h"
#define STDOUT 1

static void handle_options(char **args, int *index, int *opt_e, int *opt_n)
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

static char handle_ascii(char *str, char c)
{
    int octal = 8;
    int hexa = 16;
    char *arg = NULL;
    strcpy(arg, str + 1);
    char *error;
    char value;
    if (c == '0')
        value = strtol(str, &error, octal);
    else
        value = strtol(str, &error, hexa);
    return value;
}

static void handle_escape_aux(char *c)
{
    if (*c == 'a')
        dprintf(STDOUT, "\a");
    else if (*c == 'b')
        dprintf(STDOUT, "\b");
    else if (*c == 'e')
        dprintf(STDOUT, "%c", 27);
    else if (*c == 'f')
        dprintf(STDOUT, "\f");
    else if (*c == 'n')
        dprintf(STDOUT, "\n");
    else if (*c == 'r')
        dprintf(STDOUT, "\r");
    else if (*c == 't')
        dprintf(STDOUT, "\t");
    else if (*c == 'v')
        dprintf(STDOUT, "\v");
    else if (*c == 'x')
        dprintf(STDOUT, "%c", handle_ascii(c, *c)); //hexa
    else if (*c == '0')
        dprintf(STDOUT, "%c", handle_ascii(c, *c)); //octal
    else
        dprintf(STDOUT, "%c", *c);
}

static int print_with_backslash_escapes(char *arg)
{
    for (char *c = arg; *c != '\0'; c++)
    {
        if (*c == '\\' && *(c + 1) != '\0')
        {
            c++;
            if (*c == '\\')
                dprintf(STDOUT, "\\");
            else if (*c == 'c')
            {
                return 1;
                break;
            }
            else
            {
                handle_escape_aux(c);
                if ((*c == 'x') || (*c == '0'))
                    return 0;
            }

        }

        else
            dprintf(STDOUT, "%c", *c);
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
            dprintf(STDOUT, "%s", args[index]);

        index++;

        // print space if next arg is not NULL
        dprintf(STDOUT, "%s", args[index] != NULL ? " " : "");
    }

    // print trailing newline if option n not set
    dprintf(STDOUT, "%s", opt_n ? "" : "\n");

    return 0;
}