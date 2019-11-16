#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>

#include "parameters_handling/parameters_handler.h"
#include "parameters_handling/options.h"
#include "data_structures/queue.h"
#include "parser/parser.h"
#include "input_output/get_next_line.h"
#include "lexer/token_lexer.h"
#include "parser/ast/ast.h"
#include "error/error.h"
#include "parser/ast/destroy_tree.h"

static void sigint_handler(int signum)
{
    if (signum == SIGINT)
        printf("\n42sh$ ");
}

//the duplicated stdin/out... need to be closed at the end
static void destroy_saved_stds(void)
{
    close(10);
    close(11);
    close(12);
}

int main(int argc, char *argv[])
{
    if (handle_parameters(&g_env.options, argc, argv) == -1)
        errx(2, "invalid option or file");

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        errx(1, "an error occured while setting up a signal handler");

    int is_end = 0;
    struct queue *lexer = queue_init();
    int error = 0;
    int return_code = 0;

    while (42)
    {
        g_env.prompt = "42sh$ ";
        struct instruction *ast = parse_input(lexer, &is_end, &error);
        return_code = execute_ast(ast);
        destroy_tree(ast);
        if (is_end)
            break;
        else if (error)
        {
            handle_parser_errors(lexer);
            error = 0; //set error back to 0 for interactive mode
        }
    }

    free(lexer);
    destroy_saved_stds();

    if (is_interactive())
        puts("");

    return return_code;
}
