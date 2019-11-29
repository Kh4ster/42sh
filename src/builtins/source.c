#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include "../parser/parser.h"
#include "../parser/ast/ast.h"
#include "../parser/ast/destroy_tree.h"
#include "../data_structures/queue.h"
#include "../error/error.h"
#include "../input_output/get_next_line.h"

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


extern void execute_ressource_file(char *name, int is_source, int *error)
{
    int fd = open(name, O_RDONLY);
    if (fd != -1)
    {
        int fd_saved = dup(0);
        dup2(fd, 0);
        close(fd);
        execute_shell();
        dup2(fd_saved, 0);
        close(fd_saved);
    }
    else if (is_source)
    {
        *error = 1;
        warn("Could not open %s", name);
    }
}


extern int source(char **argv)
{
    if (!argv[1])
    {
        warnx("source: filename argument required");
        return 2;
    }
    g_env.is_parsing_ressource = 1;
    int error = 0;
    FILE * dev_null = fopen("/dev/null", "w");
    rl_outstream = dev_null;
    execute_ressource_file(argv[1], 1, &error);
    fclose(dev_null);
    rl_outstream = stdout;
    g_env.is_parsing_ressource = 0;

    if (error)
        return 2;

    return 0;
}
