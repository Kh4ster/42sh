#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fnmatch.h>

#include "../parser/ast/ast.h"
#include "../input_output/get_next_line.h"
#include "../execution_handling/command_container.h"
#include "expand_special_variables.h"


static char *expand_all()
{
    if (g_env.argc < 1)
        return strdup("");

    char *expand = strdup(g_env.argv[1]);

    for (int i = 2; i <= g_env.argc; i++)
    {
        char *new = NULL;
        int error = asprintf(&new, "%s %s", expand, g_env.argv[i]);

        if (error == -1)
            return NULL;

        free(expand);
        expand = new;
    }

    return expand;
}


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
    {
        if (g_env.old_pwd)
            return strdup(g_env.old_pwd);
        return strdup("");
    }

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

    if (strcmp("$#", variable) == 0)
    {
        int return_value = asprintf(&new_var, "%d", g_env.argc);
        if (return_value != -1)
            return new_var;
        return NULL;
    }

    if (fnmatch("$[0-9]*", variable, 0) == 0)
    {
        if (is_interactive() && strcmp(variable, "$0") == 0)
            return strdup("42sh");

        if (g_env.argc >= atoi(variable + 1))
            return strdup(g_env.argv[atoi(variable + 1)]);
        return strdup("");
    }

    if (strcmp("$@", variable) == 0 || strcmp("$*", variable) == 0)
        return expand_all();

    return NULL;
}
