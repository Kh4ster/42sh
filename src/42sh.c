#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>

#include "parameters_handling/parameters_handler.h"
#include "parameters_handling/options.h"

static int is_interactive(void)
{
    int tty = rl_instream ? fileno(rl_instream) : fileno(stdin);

    return isatty(tty);
}

static void prep_terminal(int meta_flag)
{
    if (is_interactive())
        rl_prep_terminal(meta_flag);
}

static char *get_next_line(const char *prompt)
{
    rl_prep_term_function = prep_terminal;

    if (!is_interactive())
        prompt = NULL;

    return readline(prompt);
}

static char *get_prompt(void)
{
    return "42sh$ ";
}

static void handle_line(char *line)
{
    (void)line;
    return;
}

int main(int argc, char *argv[])
{
    struct boot_params options;
    if (handle_parameters(&options, argc, argv) == -1)
        return 2;

    char *line = NULL;
    while ((line = get_next_line(get_prompt())) != NULL)
    {
        handle_line(line);
    }
    return 0;
}
