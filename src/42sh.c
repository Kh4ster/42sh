#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#include "data_structures/options.h"
#include "42sh.h"

static void print_prompt(void)
{
    printf("42sh$ ");
}

static void handle_line(char *line)
{
    (void)line;
    return;
}

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
    }
    return 1;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    char *line = NULL;
    size_t read = 0;
    print_prompt();
    while (getline(&line, &read, stdin) != -1)
    {
        handle_line(line);
        print_prompt();
    }
}
