#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>

#include "42sh.h"
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

static char *get_next_line(struct shell_environment *env, const char *prompt)
{
    /*
    ** if option c, returns command given to c
    ** set to null to exit loop at next iteration
    */
    if (env->options.option_c)
    {
        char *command = env->options.command_option_c;  
        env->options.command_option_c = NULL;
        return command;
    }

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
    printf("%s$\n", line);
    return;
}

int main(int argc, char *argv[])
{
    struct shell_environment env;
    if (handle_parameters(&env.options, argc, argv) == -1)
        return 2;

    char *line = NULL;
    while ((line = get_next_line(&env, get_prompt())) != NULL)
    {
        handle_line(line);
    }
    return 0;
}
