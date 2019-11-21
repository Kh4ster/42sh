#include <string.h>
#include <stdio.h>
#include <readline/history.h>

#include "history.h"

static int is_number(char *data)
{
    for (size_t i = 0; data[i]; i++)
    {
        if (data[i] < '0' || data[i] > '9')
            return 0;
    }

    return 1;
}

int handle_options(char **args, char *file_path)
{
    // handle options
    int r_opt = 0;
    int c_opt = 0;
    for (char *arg = *args; arg != NULL; arg++)
    {
        if (arg[0] == '-')
        {
            if (arg[1] == 'c')
                c_opt = 1;
            else if (arg[1] == 'r')
                r_opt = 1;
            else
            {
                char *usage =
                    "history: usage: history [-c] [-r] or history [n]";
                fprintf(stderr, "history: -%c: invalid option\n%s", arg[1]);
                fprintf(stderr, "%s", usage);
                return 2;
            }
        }
    }
    if (c_opt)
        clear_history();
    else if (r_opt)
        return read_history(file_path);
    return 0;
}

int show_history(char *file_path, int nb_lines)
{
    FILE *history_file = fopen(file_path, "r");
    if (history_file == NULL)
    {
        fprintf(stderr, "history: couldn't open %s", file_path);
        return 1;
    }
    char c = fgetc(history_file);
    while (c != EOF && nb_lines != 0)
    {
        fprintf(stdout, "%c", c);
        c = fgetc(history_file);
        if (c == '\n' || c == EOF)
            nb_lines -= 1;
    }
    if (c == '\n')
        fprintf(stdout, "%c", c);
    fclose(history_file);
    return 0;
}

int history(char **args)
{
    char *history_path = "~/.42sh_history";

    if (args[1] == NULL)
    {
        //display full history
        return show_history(history_path, -1);
    }

    char *first_arg = args[1];
    if (first_arg[0] == '-')
    {
        return handle_options(args, history_path);
    }
    else if (is_number(first_arg))
    {
        if (args[2] == NULL)
            return show_history(history_path, atoi(first_arg));
        else
        {
            fprintf(stderr, "history: too many arguments");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "history: numeric argument required");
        return 1;
    }

    return 0;
}