/*
** Coder : nicolas.blin & zakaria.ben-allal
** Tester : zakaria.ben-allal
** Reviewer :
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

/*
** Init command contain with command and params
** va args allows for call like init(command, param1, param2)
*/
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

/*
* doesn't free the string
* the strings might not have been allocated by malloc
*/
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
