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

int handle_options(struct array_list *args, char *file_path)
{
    // handle options
    int r_opt = 0;
    int c_opt = 0;
    for (size_t i = 0; i < args->nb_element; i++)
    {
        char *arg = args->content[i];
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
                errx(2, "history: -%c: invalid option\n%s", arg[1], usage);
            }
        }
    }
    if (c_opt)
        clear_history();
    else if (r_opt)
        read_history(file_path);
}

void show_history(char *file_path, int nb_lines)
{
    FILE *history_file = fopen(file_path, "r");
    if (history_file == NULL)
        errx(1, "history: couldn't open %s", file_path);
    char c = fgetc(history_file);
    while (c != EOF && nb_lines != 0)
    {
        while (c != EOF && c != '\n')
        {
            printf("%c", c);
        }
        nb_lines -= 1;
    }
    fclose(history_file);
}

int history(struct array_list *args)
{
    char *history_path = "~/.42sh_history";

    if (args->nb_element == 0)
    {
        //display full history
        show_history(history_path, -1);
    }

    char *first_arg = args->content[0];
    if (first_arg[0] == '-')
    {
        handle_options(args, history_path);
    }
    else if (is_number(first_arg))
    {
        if (args->nb_element == 1)
            show_history(history_path, atoi(first_arg));
        else
            errx(1, "history: too many arguments");
    }
    else
        errx(1, "history: numeric argument required");

    return 0;
}