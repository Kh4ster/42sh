#include <getopt.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "parameters_handler.h"
#include "options.h"
#include "../execution_handling/command_container.h"
#include "../builtins/shopt.h"

static int handle_file(char *file)
{
    int fd = -1;

    if ((fd = open(file, O_RDONLY)) == -1)
        return -1;

    if (dup2(fd, 0) == -1)
        return -1;

    close(fd);
    return 0;
}

/*
** Usually the shopt is a call to a builtin with some parameters
** Here the builin is "called" through a [+-]O option passed to the shell
** To avoid wirting again the same code a "fake" shopt command is created
** and then executed
*/
static int build_shopt_call(bool set, char *option)
{
    struct command_container *shopt_container = NULL;

    if (set && option != NULL)       //set option
        shopt_container = command_init(2, "shopt", "-s", option);

    else if (!set && option != NULL) //unset option
        shopt_container = command_init(2, "shopt", "-u", option);

    else                            //just call shopt
         shopt_container = command_init(0, "shopt");

    int return_value = shopt(shopt_container->params);

    command_destroy(&shopt_container);
    optind++;

    return return_value;
}

// a bad option can be a +O option or a file or juste a bad option
static int handle_not_existing_option(char *argv[])
{
    char *current_option = argv[optind];
    if (strcmp(current_option, "+O") == 0)
    {
        if (build_shopt_call(false, optarg) == -1)
            return -1;
    }
    else
    {
        if (handle_file(current_option) == -1)
            return -1;
    }
    optind++;
    return 0;
}

int handle_parameters(struct boot_params *options,
                        int argc,
                        char *argv[]
)
{
    char c;
    static const struct option long_opts[] =
    {
        {"ast-print", no_argument, NULL, 'A'},
        {"norc", no_argument, NULL, 'N'}
    };

    while (optind < argc)
    {
        c = getopt_long(argc, argv, "NAOc:", long_opts, NULL);
        if (c  == -1)
        {
            if (handle_not_existing_option(argv) == -1)
                return -1;
        }
        else if (c == 'N')
            options->option_n = true;
        else if (c == 'A')
            options->option_a = true;
        else if (c == 'O')
        {
            if (build_shopt_call(true, optarg) == -1)
                return -1;
        }
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
    optind = 1; //set back to 1 for futur get_opt call
    return 1;
}