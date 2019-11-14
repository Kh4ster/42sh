#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>

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

int main(int argc, char *argv[])
{

    if (handle_parameters(&g_env.options, argc, argv) == -1)
        return 2;

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        err(1, "an error occured while setting up a signal handler");

    int is_end = 0;
    struct queue *lexer = queue_init();

    while (42)
    {
        g_env.prompt = "42sh$ ";
        struct instruction *ast = parse_input(lexer, &is_end);
        execute_ast(ast);
        destroy_tree(ast);
        if (is_end)
            break;
        else if (ast == NULL)
            handle_parser_errors(lexer);
    }
    free(lexer);

    //puts("");
    return 0;
}
