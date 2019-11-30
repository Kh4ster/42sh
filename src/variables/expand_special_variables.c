#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../parser/ast/ast.h"
#include "../input_output/get_next_line.h"
#include "../execution_handling/command_container.h"
#include "expand_special_variables.h"


char *expand_special_variables(char *variable)
{
    char *new_var = NULL;

    if (strcmp("$?", variable) == 0)
    {
        int return_value = asprintf(&new_var, "%d", g_env.last_return_value);
        if (return_value != -1)
            return new_var;
        return NULL;
    }

    if (strcmp("$OLDPWD", variable) == 0)
        return strdup(g_env.old_pwd);

    if (strcmp("$IFS", variable) == 0)
        return strdup(getenv("IFS"));

    if (strcmp("$$", variable) == 0)
    {
        int return_value = asprintf(&new_var, "%d", getpid());
        if (return_value != -1)
            return new_var;
        return NULL;
    }

    if (strcmp("$RANDOM", variable) == 0)
    {
        int return_value = asprintf(&new_var, "%d", rand());
        if (return_value != -1)
            return new_var;
        return NULL;
    }

    if (strcmp("$UID", variable) == 0)
    {
        int return_value = asprintf(&new_var, "%d", getuid());
        if (return_value != -1)
            return new_var;
        return NULL;
    }

    return NULL;
}
