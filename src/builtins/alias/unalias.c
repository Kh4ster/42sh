#include <stdio.h>
#include <err.h>
#include <string.h>

#include "unalias.h"
#include "../../data_structures/hash_map.h"
#include "../../input_output/get_next_line.h"

static void print_usage(void)
{
    warnx("usage: unalias [-a] name [name...]");
}


static int handle_unalias(char *alias)
{
    if (!hash_remove(g_env.aliases, alias))
        return 1;

    return 0;
}


int unalias(char **args)
{
    if (!args[1])
    {
        print_usage();
        return 0;
    }

    if (strcmp(args[1], "-a") == 0)
    {
        hash_free(g_env.aliases);
        struct hash_map new_aliases;
        g_env.aliases = &new_aliases;
        hash_init(&new_aliases, 512);
        return 0;
    }

    int to_return = 0;

    for (size_t i = 0; args[i]; i++)
    {
        int new_return = handle_unalias(args[i]);

        if (to_return == 0)
            to_return = new_return;
    }

    return to_return;
}
