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
#include "builtins/cd.h"

static void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        printf("\n42sh$ ");
    }
}


static void is_noclobber(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "+o") == 0)
        {
            if (i < argc - 1 && strcmp("noclobber", argv[i + 1]) == 0)
            {
                g_env.noclobber_set = 0;
                return;
            }

            if (i == argc - 1)
            {
                puts("set +o noclobber");
                return;
            }
            errx(2, "Invalid option");
        }

        if (strcmp(argv[i], "-o") == 0)
        {
            if (i < argc - 1 && strcmp("noclobber", argv[i + 1]) == 0)
            {
                g_env.noclobber_set = 1;
                return;
            }

            if (i == argc - 1)
            {
                if (g_env.noclobber_set)
                    puts("noclobber    on");
                else
                    puts("noclobber    off");
                return;
            }

            errx(2, "invalid option");
        }

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

void end_call_and_free_all(struct queue *lexer)
{
    // Have a \n on ctrl-d in interactive mode
    if (is_interactive())
        puts("");

    // Free all
    hash_free(g_env.functions);
    hash_free(g_env.builtins);
    free(g_env.pwd);
    free(lexer);

    // History write to file
    char *history_path = get_history_file_path();
    FILE *history_file = fopen(history_path, "w");
    if (history_file == NULL)
    {
        free(history_path);
        return;
    }
    HIST_ENTRY **hist_elts = history_list();
    for (int i = 0; i < history_length - 1; i++)
        fprintf(history_file, "%s\n", hist_elts[i]->line);
    fclose(history_file);
    history_truncate_file(history_path, 2000);
    free(history_path);
}

static void init_builtins_hash_map(struct hash_map *builtins)
{
    hash_init(builtins, NB_SLOTS);
    hash_insert_builtin(builtins, "shopt", shopt);
    hash_insert_builtin(builtins, "history", history);
    hash_insert_builtin(builtins, "cd", cd);
}

static void init_hash_maps_and_history(struct hash_map *functions,
                            struct hash_map *builtins
)
{
    // Hash map
    g_env.functions = functions;
    g_env.builtins = builtins;
    hash_init(functions, NB_SLOTS);
    init_builtins_hash_map(g_env.builtins);
    g_env.options.option_expand_aliases = true;
    g_env.options.option_sourcepath = true;

    // History
    char *history_path = get_history_file_path();
    using_history();
    read_history(history_path);
    free(history_path);
}

static void handle_signal(void)
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        errx(1, "an error occurred while setting up a signal handler");
}

static int execute_and_print_ast(struct instruction *ast)
{
    if (g_env.options.option_a)
        print_ast(ast);

    return execute_ast(ast);
}

int main(int argc, char *argv[])
{
    struct hash_map functions; //declared on the stack no need to be freed
    struct hash_map builtins;
    init_hash_maps_and_history(&functions, &builtins);

    int return_code = 0;

    if ((return_code = handle_parameters(&g_env.options, argc, argv)) != 0)
        errx(return_code, "invalid option or file");

    is_noclobber(argc, argv);
    handle_signal();

    handle_ressource_files();

    int is_end = 0;
    struct queue *lexer = queue_init();

    while (42 && !is_end)
    {
        int error = 0;
        g_env.prompt = "42sh$ ";
        struct instruction *ast = parse_input(lexer, &is_end, &error);

        if (ast)
            return_code = execute_and_print_ast(ast);

        destroy_tree(ast);

        handle_signal();

        if (!is_end && error)
            handle_parser_errors(lexer);
    }

    end_call_and_free_all(lexer);

    return return_code;
}
