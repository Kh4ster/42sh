#include <string.h>

#include "history.h"

static int is_number(char *data)
{
    for (size_t i = 0; data[i]; i++)
    {
        if (data[i] < '0' || data[i] > '9')
            return 0;
    }

    return 1;
}

int history(struct array_list *args)
{
    if (args->nb_element == 0)
    {
        // return : display all history file
    }
    char *first_arg = args->content[0];
    if (first_arg[1] == '-')
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
                    err(99, "history: -%c: invalid option", arg[1]);
            }
        }
        if (c_opt)
            //history_clear()
        else if (r_opt)
            //history_restore()
    }
    else if (is_number(first_arg))
    {
        if (args->nb_element == 1)
        {
            // return : display n lines of history file
        }
        else
        {
            /* code */
        }
    }
}