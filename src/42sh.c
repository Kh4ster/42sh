#include <stdio.h>
#include <stdlib.h>

#include "parameters_handling/options.h"

static void print_prompt(void)
{
    printf("42sh$ ");
}

static void handle_line(char *line)
{
    (void)line;
    return;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    char *line = NULL;
    size_t read = 0;
    print_prompt();
    while (getline(&line, &read, stdin) != -1)
    {
        handle_line(line);
        print_prompt();
    }
}
