#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <err.h>

#include "../memory/memory.h"
#include "export.h"
#include "../input_output/get_next_line.h"

/*
static char *var_name(char *arg)
{
    int i = 0;
    char *varname = NULL;
    if (strchr(arg, '=') != NULL)
    {
        while (arg[i] != '=')
        {
            varname[i] = arg[i];
            i++;
        }
        varname[i] = '\0'; 
    }
    return varname;
}

static char *var_value(char *arg)
{
    int i = 0;
    char *varvalue = NULL;
    if (strchr(arg, '=') != NULL)
    {
        while (arg[i] != '=')
            i++;
        i++;
        while (arg[i])
        {
            varvalue[i] = arg[i];
            i++;
        }
        varvalue[i] = '/0';
    }
    return varvalue;
}
*/

static void free_env(char **env, int k)
{
    for (; env[k]; k++)
    {
        free(env[k]);
        env[k] = NULL;
    }
}

static void simple_export(char **env)
{
    for(int i = 0; env[i] != NULL; i++)
        printf("%s\n", env[i]);
}

static void save_old_env(void)
{
    size_t i = 0;
    while (g_env.envvar[i] != NULL)
        i++;
    g_env.old_envvar = xcalloc(i + 1, sizeof(char *));
    i = 0;
    for (; g_env.envvar[i] != NULL; i++)
    {
        g_env.old_envvar[i] = strdup(g_env.envvar[i]);
    }
    g_env.old_envvar[i] = NULL;
}

static void n_export(char *arg)
{
    int i = 0;
    int j = 0;
    int k = 0;
    free_env(g_env.envvar, k);

    while (strncmp(g_env.old_envvar[i], arg, strlen(arg)) != 0) //TOTO resolve TERM CONFLICT
    {
        g_env.envvar[j] = strdup(g_env.old_envvar[i]);
        i++;
        j++;

        if (g_env.old_envvar[i] == NULL)
        {
            free_env(g_env.envvar, j); //free spare room
            return;
        }
    }
    i++;
    while (g_env.old_envvar[i] != NULL)  
    {
        g_env.envvar[j] = strdup(g_env.old_envvar[i]);
        i++;
        j++;        
    }
    free_env(g_env.envvar, j); //free spare room
}

static int handle_error(char **arg)
{
    for (int i = 0; arg[i]; i++)
    {
        if (strcmp(arg[i], "=") == 0)
        {
            fprintf(stderr, "export: '=' not a valid identifier\n");
            return 1;
        }
    }
    return 0;
}


int export(char **argv)
{
    assert(argv && (strcmp(argv[0], "export") == 0));
    if (handle_error(argv) == 1) //check if syntax error
        return 1;
    int argc = 0;
    while (argv[argc] != NULL)
        argc++;

    if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-p") == 0)))
        simple_export(g_env.envvar);

    if (argc > 2 && (strcmp(argv[1], "-n")  == 0))
    {
        int i = 2;
        while (argv[i])
        {
            //handle_error(argv[2]); //handle syntax error
            if (g_env.old_envvar != NULL)
            {
                for (int i = 0; g_env.old_envvar[i]; i++)
                    free(g_env.old_envvar[i]);
            }
            free(g_env.old_envvar);
            save_old_env();
            n_export(argv[i]);
            int j = 0;
            free_env(g_env.old_envvar, j);
            free(g_env.old_envvar);
            save_old_env();
            i++;
        }
        return 0;
    }
    return 0;  
}
