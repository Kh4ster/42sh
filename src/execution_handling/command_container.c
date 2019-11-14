/*
** Coder : nicolas.blin & zakaria.ben-allal
** Tester : zakaria.ben-allal
** Reviewer : nicolas.blin
** Integrator :
**
** Container for commands
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "command_container.h"
#include "../memory/memory.h"
#include "../data_structures/array_list.h"
#include "command_execution.h"

struct command_container* command_init(size_t nb_args, char *command, ...)
{
    va_list ap;
    va_start(ap, command);

    struct command_container *command_container;
    command_container = xmalloc(sizeof(struct command_container));
    command_container->command = command;
    command_container->params = xmalloc(sizeof(char*) * (nb_args + 2));
    command_container->params[0] = command;
    char *end = command_container->params[0] + strlen(command_container->params[0]);
    while (end - command_container->params[0] >= 0 && *end != '/')
        end--;
    end++;
    command_container->params[0] = end;
    for (size_t i = 0; i < nb_args; ++i)
        command_container->params[i + 1] = va_arg(ap, char*);
    command_container->params[nb_args + 1] = NULL;

    va_end(ap);

    return command_container;
}

void command_destroy(struct command_container **ptr_command_container)
{
    struct command_container *command_container = *ptr_command_container;
    free(command_container->params);
    free(command_container);
    *ptr_command_container = NULL;
}

struct command_container *command_create(char *cmd, struct array_list *list)
{
    struct command_container *save;
    save = xcalloc(1, sizeof(struct command_container));
    save->params = xcalloc(list->nb_element + 2, sizeof(char *));
    save->command = xcalloc(strlen(cmd) + 1, sizeof(char));
    strcpy(save->command, cmd);
    save->params[0] = cmd;
    size_t j = 0;
    for (; j < list->nb_element; j++)
    {
        save->params[j + 1] = list->content[j];
    }
    save->params[j + 1] = NULL;
    return save;
}
