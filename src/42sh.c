#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <readline/readline.h>
#include <unistd.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parameters_handling/parameters_handler.h"
#include "parameters_handling/options.h"
#include "data_structures/queue.h"
#include "parser/parser.h"
#include "input_output/get_next_line.h"
#include "lexer/token_lexer.h"
#include "parser/ast/ast.h"
#include "error/error.h"
#include "parser/ast/destroy_tree.h"
#include "parser/ast/ast_print.h"
#include "memory/memory.h"
#include "data_structures/hash_map.h"
#include "builtins/shopt.h"
#include "builtins/history.h"

static void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        printf("\n42sh$ ");
    }
}

static void execute_shell(void)
{
    int is_end = 0;
    struct queue *lexer = queue_init();
    int error = 0;

    while (42)
    {
        g_env.prompt = "42sh$ ";
        struct instruction *ast = parse_input(lexer, &is_end, &error);
        execute_ast(ast);
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
}

static void execute_ressource_file(char *name)
{
    int fd = open(name, O_RDONLY);
    if (fd != -1)
    {
        dup2(0, 10);
        dup2(fd, 0);
        close(fd);
        execute_shell();
        dup2(10, 0);
        close(10);
    }
}

static void handle_ressource_files(void)
{
    if (!g_env.options.option_n)
    {
        g_env.is_parsing_ressource = 1;
        execute_ressource_file("/etc/42shrc");
        char *s = getenv("HOME");
        char *full_path = xmalloc(strlen(s) + strlen("/.42shrc") + 1);
        strcpy(full_path, s);
        strcat(full_path, "/.42shrc");
        execute_ressource_file(full_path);
        free(full_path);
        g_env.is_parsing_ressource = 0;
    }
}

void free_all(struct queue *lexer)
{
    hash_free(g_env.functions);
    hash_free(g_env.builtins);
    free(lexer);
}

static void init_builtins_hash_map(struct hash_map *builtins)
{
    hash_init(builtins, NB_SLOTS);
    hash_insert_builtin(builtins, "shopt", shopt);
    hash_insert_builtin(builtins, "history", history);
}

static void init_hash_maps(struct hash_map *functions,
                            struct hash_map *builtins
)
{
    g_env.functions = functions;
    g_env.builtins = builtins;
    hash_init(functions, NB_SLOTS);
    init_builtins_hash_map(g_env.builtins);
}

int main(int argc, char *argv[])
{
    if (handle_parameters(&g_env.options, argc, argv) == -1)
        errx(2, "invalid option or file");

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        errx(1, "an error occurred while setting up a signal handler");

    struct hash_map functions; //declared on the stack no need to be freed
    struct hash_map builtins;
    init_hash_maps(&functions, &builtins);

    handle_ressource_files();

    int is_end = 0;
    struct queue *lexer = queue_init();
    int error = 0;
    int return_code = 0;

    while (42)
    {
        g_env.prompt = "42sh$ ";
        struct instruction *ast = parse_input(lexer, &is_end, &error);

        if (g_env.options.option_a)
            print_ast(ast);

        return_code = execute_ast(ast);

        destroy_tree(ast);

        if (signal(SIGINT, sigint_handler) == SIG_ERR)
            errx(1, "an error occurred while setting up a signal handler");

        if (is_end)
            break;

        else if (error)
        {
            handle_parser_errors(lexer);
            error = 0; //set error back to 0 for interactive mode
        }
    }

    free_all(lexer);

    if (is_interactive()) //to have a \n on ctrl-d in interactive mode
        puts("");

    return return_code;
}
