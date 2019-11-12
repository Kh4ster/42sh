#include "command_storage.h"
#incluce "../memory/memory.h"

struct command_container cmd_store(char *line)
{
    struct command_container *save = xcalloc(1, sizeof(struct command_container));
    if (!save)
        return NULL;
    return save;
}
