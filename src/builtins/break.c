#include <stdlib.h>
#include "../input_output/get_next_line.h"

int has_continue(char **argv)
{
    if (!argv[1])
        g_env.continues = 1;
    else
        g_env.continues = atoi(argv[1]);

    return 0;
}

int has_break(char **argv)
{
    if (!argv[1])
        g_env.breaks = 1;
    else
        g_env.breaks = atoi(argv[1]);

    return 0;
}
