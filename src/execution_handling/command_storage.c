#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command_storage.h"
#include "../memory/memory.h"
#include "../data_structures/array_list.h"
#include "command_execution.h"

struct command_container *command_create(char *cmd, struct array_list *list)
{
    struct command_container *save;
    save = xcalloc(1, sizeof(struct command_container));
    save->params = xcalloc(list->max_size + 2, sizeof(char *));
    save->command = xcalloc(256, sizeof(char));
    int i = 0;
    for (; cmd[i] != ' ' && cmd[i] != '\n' && cmd[i]; i++)
    {
        save->command[i] = cmd[i];
    }
    save->command[i] = 0;
    size_t j = 1;
    size_t k = 0;
    save->params[0] = cmd;
    while (k < list->nb_element)
    {
        save->params[j] = list->content[k];
        j++;
        k++;
    }
    save->params[j] = NULL;
    return save;
}
/*
int main()
{
    char *cmd = strdup("echo");
    struct array_list *list = init_array_list();
    append_array_list(list, strdup("toto"));
    struct command_container *c = cmd_store(cmd, list);
    int exec = exec_cmd(c);
    free(c->command);
    command_destroy(&c);
    destroy_array_list(list);
    return exec;
}

*/
