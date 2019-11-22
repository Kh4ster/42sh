#include <string.h>
#include <stdio.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "history.h"
#include "../memory/memory.h"

static char *get_homedir(void)
{
    char *homedir;
    if ((homedir = strdup(getenv("HOME"))) == NULL)
    {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd == NULL)
            return NULL;
        homedir = strdup(pwd->pw_dir);
    }
    return homedir;
}

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
    for (int i = 1; args[i] != NULL; i++)
    {
        char *arg = args[i];
        if (arg[0] == '-')
        {
            if (arg[1] == 'c')
                c_opt = 1;
            else if (arg[1] == 'r')
                r_opt = 1;
            else
            {
                char *usage =
                    "history: usage: history [-c] [-r] or history [n]\n";
                fprintf(stderr, "history: -%c: invalid option\n", arg[1]);
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
        fprintf(stderr, "history: couldn't open %s\n", file_path);
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
    int return_val = 0;
    char *history_name = "/.42sh_history";
    char *homedir = get_homedir();
    homedir = xrealloc(homedir, strlen(homedir) + 1 + strlen(history_name));
    char *history_path = strcat(homedir, history_name);

    if (args[1] == NULL)
    {
        //display full history
        return_val = show_history(history_path, -1);
    }

    else
    {
        char *first_arg = args[1];
        if (first_arg[0] == '-')
        {
            return_val = handle_options(args, history_path);
        }
        else if (is_number(first_arg))
        {
            if (args[2] == NULL)
                return_val = show_history(history_path, atoi(first_arg));
            else
            {
                fprintf(stderr, "history: too many arguments\n");
                return_val = 1;
            }
        }
        else
        {
            fprintf(stderr, "history: %s: numeric argument required\n",
                    first_arg);
            return_val = 1;
        }
    }

    free(history_path);
    return return_val;
}