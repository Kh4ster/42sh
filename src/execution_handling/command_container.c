/*
** Coder : nicolas.blin
** Tester :
** Reviewer :
** Integrator :
**
** Container for commands
*/

#include <stdlib.h>
#include <stdarg.h>

#include "command_container.h"
#include "../memory/memory.h"

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
    for (size_t i = 0; i < nb_args; ++i)
        command_container->params[i + 1] = va_arg(ap, char*);
    command_container->params[nb_args + 1] = NULL;

    va_end(ap);

    return command_container;
}

/*
* does't free the string
* the strings might not have been alocated by malloc
*/ 
void command_destroy(struct command_container **ptr_command_container)
{
    struct command_container *command_container = *ptr_command_container;
    free(command_container->params);
    free(command_container);
    *ptr_command_container = NULL;
}