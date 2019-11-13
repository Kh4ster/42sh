#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>

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

static void handle_line(char *line)
{
    printf("%s$\n", line);
    add_history(line);
}

static char *get_next_line(struct shell_environment *env, const char *prompt)
{
    // if option c, returns NULL to exit the execution loop
    if (env->options.option_c)
    {
        char *command = env->options.command_option_c;
        env->options.command_option_c = NULL;
        handle_line(command);
        return NULL;
    }

    rl_prep_term_function = prep_terminal;

    if (!is_interactive())
        prompt = NULL;

    return readline(prompt);
}

static char *get_prompt(void)
{
  //  if (is_line_finished(line))
        return "42sh$ ";
 //   else
 //       return "> ";
}


static void sigint_handler(int signum)
{
    if (signum == SIGINT)
        printf("\n42sh$ ");
}


int main(int argc, char *argv[])
{
    struct shell_environment env = {0};

    if (handle_parameters(&env.options, argc, argv) == -1)
        return 2;

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        err(1, "an error occured while setting up a signal handler");

    char *line = get_next_line(&env, get_prompt());

    while (line)
    {
        handle_line(line);
        free(line);
        line = get_next_line(&env, get_prompt());
    }
    puts("");
    return 0;
}
