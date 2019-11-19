#include <string.h>

#include "history.h"

int history(struct array_list *args)
{
    // handle options
    int r_opt = 0;
    int c_opt = 0;
    for (size_t i = 0; i < args->nb_element; i++)
    {
        char *arg = args->content[i];
        if (arg[0] == '-')
        {
            if (arg[1] == 'c')
                c_opt = 1;
            else if (arg[1] == 'r')
                r_opt = 1;
            else
                err("history: -%c: invalid option", arg[1]);
        }
        if (is_num_arg)
    }
}