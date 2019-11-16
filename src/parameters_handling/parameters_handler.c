#include <getopt.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "parameters_handler.h"
#include "options.h"

int handle_parameters(struct boot_params *options,
                        int argc,
                        char *argv[]
)
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
        {
            char *current_option = argv[optind];
            if (strcmp(current_option, "+O") == 0)
                options->option_o = true;
            else
            {
                if (handle_file(current_option) == -1)
                    return -1;
            }
            optind++;
        }
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

int handle_file(char *file)
{
    int fd = -1;
    if ((fd = open(file, O_RDONLY)) == -1)
        return -1;
    if (dup2(fd, 0) == -1)
        return -1;
    close(fd);
    return 0;
}
