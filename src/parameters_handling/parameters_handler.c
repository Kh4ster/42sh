#include <getopt.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "parameters_handler.h"
#include "../input_output/get_next_line.h"
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

static void print_pipable_shopt(void)
{
    static const char *options[] =
    {
        "ast_print",
        "dotglob",
        "expand_aliases",
        "extglob",
        "nocaseglob",
        "nullglob",
        "sourcepath",
        "xpg_echo"
        "noclobber"
    };
    static const size_t size_array = sizeof(options) / sizeof(char*);

    bool val_options[] =
    {
        g_env.options.option_a,
        g_env.options.option_dot_glob,
        g_env.options.option_expand_aliases,
        g_env.options.option_extglob,
        g_env.options.option_nocaseglob,
        g_env.options.option_nullglob,
        g_env.options.option_sourcepath,
        g_env.options.option_xpg_echo
    };

    for (size_t i = 0; i < size_array; ++i)
        printf("shopt %s %s\n", (val_options[i]) ? "-s" : "-u", options[i]);
}

/*
** Usually the shopt is a call to a builtin with some parameters
** Here the builtin is "called" through a [+-]O option passed to the shell
** To avoid writing again the same code a "fake" shopt command is created
** and then executed
*/
static int build_shopt_call(bool set, char *option)
{
    struct command_container *shopt_container = NULL;

    if (set && option != NULL)       //set option
    {
        shopt_container = command_init(2, "shopt", "-s", option);
        optind++;
    }

    else if (!set && option != NULL) //unset option
    {
        shopt_container = command_init(2, "shopt", "-u", option);
        optind++;
    }

    else                            //just call shopt
    {
        if (set)
            shopt_container = command_init(0, "shopt");
        else
        {
            print_pipable_shopt();
            return 0;
        }
    }

    int return_value = shopt(shopt_container->params);

    command_destroy(&shopt_container);

    return return_value;
}

/*
** 127 no file
** 0 shopt ok
** 1 file ok
** 2 bad shopt
*/
static int handle_not_existing_option(char *argv[])
{
    char *current_option = optarg;

    if (strcmp(current_option, "+o") == 0)
        return 1;

    if (strcmp(current_option, "+O") == 0)
    {
        if (build_shopt_call(false, argv[optind]) != 0)
            return 2;
    }
    else
    {
        if (handle_file(current_option) == -1)
            return 127;
        else
            return 1;
    }

    return 0;
}

static int is_N_or_A_opt(struct boot_params *options, char c)
{
    if (c == 'N')
    {
        options->option_n = true;
        return 1;
    }
    if (c == 'A')
    {
        options->option_a = true;
        return 1;
    }
    return 0;
}

static void set_c_option(struct boot_params *options)
{
    options->option_c = true;
    options->command_option_c = optarg;
}

int handle_parameters(struct boot_params *options,
                        int argc,
                        char *argv[]
)
{
    char c;
    int return_value;
    static const struct option long_opts[] =
    {
        {"ast-print", no_argument, NULL, 'A'},
        {"norc", no_argument, NULL, 'N'}
    };

    while (optind < argc)
    {
        c = getopt_long(argc, argv, "-NAO::c:o;", long_opts, NULL);
        if (c  == 1)
        {
            if ((return_value = handle_not_existing_option(argv)) == 0)
                continue; //ok
            else if (return_value == 1)
                return 0;
            else
                return return_value;
        }
        else if (!is_N_or_A_opt(options, c))
        {
            if (c == 'O')
            {
                if ((return_value = build_shopt_call(true, argv[optind])) != 0)
                    return 2;
            }
            else if (c == 'c')
            {
                if (optarg == NULL)
                    return 2;
                set_c_option(options);
            }
            else if (c == 'o')
                return 0;
            else if (c == '?')
                return 2;
        }
    }
    return 0;
}
