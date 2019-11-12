#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command_storage.h"
#include "../memory/memory.h"

struct command_container *cmd_store(char *line)
{
    struct command_container *save;
    save = xcalloc(1, sizeof(struct command_container));
    save->params = xcalloc(strlen(line) + 1, sizeof(char *));
    save->command = xcalloc(1, sizeof(char *));
    int i = 0;
    for (; line[i] != ' ' && line[i] != '\n' && line[i]; i++)
    {
        save->command[i] = line[i];
    }
    save->command[i] = 0;
    strcpy(save->params[0], line);
    return save;
}

int main()
{
    char *line = "echo toto";
    struct command_container *save = cmd_store(line);
    printf("%s\n", save->command);
    printf("%s\n", save->params[0]);
    command_destroy(&save);
    return 0;
}
