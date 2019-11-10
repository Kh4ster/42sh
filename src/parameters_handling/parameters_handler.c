/*
** Coder : nicolas.blin
** Tester :
** Reviewer :
** Integrator :
**
** Handle entry parameters given to 42sh binary
*/

#include <getopt.h>
#include <stddef.h>

#include "parameters_handler.h"
#include "options.h"

int handle_parameters(struct boot_params *options, int argc, char *argv[])
{
    char c;
    struct option long_opts[] =
    {
        {"ast-print", no_argument, NULL, 'A'},
        {"norc", no_argument, NULL, 'N'}
    };

    while (optind < argc)
    {
        c = getopt_long(argc, argv, "NAOc:", long_opts, NULL);
        if (c  == -1)
            return -1;
        else if (c == 'N')
            options->option_n = true;
        else if (c == 'A')
            options->option_a = true;
        else if (c == 'O')
            options->option_o = true;
        else if (c == 'c')
        {
            if (optarg == NULL)
                return -1;
            options->option_c = true;
            options->command_option_c = optarg;
        }
        else if (c == '?')
            return -1;
    }
    return 1;
}