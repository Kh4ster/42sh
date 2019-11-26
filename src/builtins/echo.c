#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "echo.h"

int nb_args(char **args)
{
    int count = 0;
    for(; *args != NULL; args++)
        count++;

    return count;
}

int echo(int argc, char **args)
{
    char c;
    //int argc = nb_args;

    // options e, n and E of echo
    int opt_n = 0;
    int opt_e = 0; // E: 0, e: 1

    // set getopt variable (index = 0) (no error messages)
    optind = 0;
    opterr = 0;

    while (optind < argc)
    {
        c = getopt(argc, args, "Een");
        if (c == 1)
        {
            printf("test");
        }
        else if (c == 'n')
            opt_n = 1;
        else
            opt_e = (c == 'e') ? 1 : 0;
    }
    printf("opt_n = %d\nopt_e = %d\n", opt_n, opt_e);
    return 0;
}

int main(int argc, char **argv)
{
    echo(argc, argv);
}