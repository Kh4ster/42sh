#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "echo.h"
#include "../input_output/get_next_line.h"
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

static int is_hexa(char c)
{
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F'))
        return 1;
    else
        return 0;
}

static int is_octal(char c)
{
    return c >= '0' && c <= '7';
}

static char handle_ascii(char **str, char base)
{
    int octal = 8;
    int hexa = 16;
    int nb_digits = 0;
    char result;
    if (base == '0')
    {
        while(is_octal(*(*str + nb_digits + 1)) && nb_digits < 3)
            nb_digits++;

        if (nb_digits == 0)
            return 0;

        char *digit_to_convert = strndup(*str + 1, nb_digits);
        result = strtol(digit_to_convert, NULL, octal);
        free(digit_to_convert);

        // put str at the end of the ascii digits
        *str += nb_digits;
    }
    else // base == 'x'
    {
        while(is_hexa(*(*str + nb_digits + 1)) && nb_digits < 2)
            nb_digits++;

        if (nb_digits == 0)
            return 0;

        char *digit_to_convert = strndup(*str + 1, nb_digits);
        result = strtol(digit_to_convert, NULL, hexa);
        free(digit_to_convert);

        // put str at the end of the ascii digits
        *str += nb_digits;
    }
    return result;
}

static void handle_escape_aux(char **c)
{
    if (**c == 'a')
        dprintf(STDOUT, "\a");
    else if (**c == 'b')
        dprintf(STDOUT, "\b");
    else if (**c == 'e')
        dprintf(STDOUT, "%c", 27);
    else if (**c == 'f')
        dprintf(STDOUT, "\f");
    else if (**c == 'n')
        dprintf(STDOUT, "\n");
    else if (**c == 'r')
        dprintf(STDOUT, "\r");
    else if (**c == 't')
        dprintf(STDOUT, "\t");
    else if (**c == 'v')
        dprintf(STDOUT, "\v");
    else if (**c == 'x' || **c == '0')
    {
        char new_char = handle_ascii(c, **c);
        if (new_char == 0) // no corresponding ascii found
            dprintf(STDOUT, "\\%c", **c); // print \\x or \\0
        else
            dprintf(STDOUT, "%c", new_char);
    }
    else
        dprintf(STDOUT, "\\%c", **c);
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
                handle_escape_aux(&c);
            }
        }
        else // if (*c != '\\')
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

    if (! g_env.options.option_xpg_echo)
        // Set options and put the index at the next arg to print
        handle_options(args, &index, &opt_e, &opt_n);
    else // if xpg_echo is activated
        opt_e = 1;

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