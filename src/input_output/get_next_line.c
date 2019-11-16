#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>

#include "get_next_line.h"
#include "../parameters_handling/parameters_handler.h"
#include "../parameters_handling/options.h"
#include "../data_structures/queue.h"
#include "../parser/parser.h"

struct shell_environment g_env = {0};

int is_interactive(void)
{
    if (g_env.options.option_c)
        return 0;
    int tty = rl_instream ? fileno(rl_instream) : fileno(stdin);

    return isatty(tty);
}

static void prep_terminal(int meta_flag)
{
    if (is_interactive())
        rl_prep_terminal(meta_flag);
}

char *get_next_line(const char *prompt)
{
    rl_prep_term_function = prep_terminal;
    // if option c, returns NULL to exit the execution loop
    if (g_env.options.option_c)
    {
        char *command = g_env.options.command_option_c;
        g_env.options.command_option_c = NULL;
        return command;
    }

    if (!is_interactive())
        prompt = NULL;

    char *new_line = readline(prompt);
    add_history(new_line);
    return new_line;
}
