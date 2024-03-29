#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>

#include "get_next_line.h"
#include "prompt.h"
#include "../parameters_handling/parameters_handler.h"
#include "../parameters_handling/options.h"
#include "../data_structures/queue.h"
#include "../data_structures/hash_map.h"
#include "../parser/parser.h"
#include "../data_structures/data_string.h"

struct shell_environment g_env = {0};

int is_interactive(void)
{
    if (g_env.options.option_c) //otherwise terminal -c considered interactive
        return 0;

    int tty = rl_instream ? fileno(rl_instream) : fileno(stdin);

    return isatty(tty);
}

static void prep_terminal(int meta_flag)
{
    if (is_interactive())
        rl_prep_terminal(meta_flag);
}

char *get_next_line(int prompt)
{
    rl_prep_term_function = prep_terminal;

    /*
    ** if option c we don't call readline and next get_next_line should r NULL
    ** but we want to make a readline call if we are parsing the ressource file
    ** even if -c option was use to call 42sh binary
    */
    if (g_env.options.option_c && !g_env.is_parsing_ressource)
    {
        char *command = g_env.options.command_option_c;
        g_env.options.command_option_c = NULL;
        return command;
    }

    if (!is_interactive())
    {
        rl_bind_key ('\t', rl_insert);
        prompt = 0;
    }
    else
    {
        rl_bind_key ('\t', rl_complete);
    }

    char *prompt_str = get_prompt(prompt);
    char *new_line = readline(prompt_str);
    free(prompt_str);

    if (!g_env.is_parsing_ressource)
        add_history(new_line);

    return new_line;
}
