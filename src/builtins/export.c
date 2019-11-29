#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <err.h>

#include "../memory/memory.h"
#include "export.h"
#include "../input_output/get_next_line.h"


/*
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
static int handle_error(int argc, char **arg)
{
    if (argc > 2 && arg[1] != NULL &&  (strcmp(arg[1], "-n") == 0) && (strcmp(arg[2], "-p") != 0))
    {
        for (int i = 3; arg[i]; i++)
        {
            if (strcmp(arg[i], "-p") == 0)
            {
                fprintf(stderr, "export: '-p' not a valid identifier\n");
                return 1;
            }
        }
    }
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

static void save_old_env(void)
{
    size_t i = 0;
    while (g_env.envvar[i] != NULL)
        i++;

    g_env.old_envvar = xcalloc(i + 1, sizeof(char *));

    for (i = 0; g_env.envvar[i] != NULL; i++)
    {
        g_env.old_envvar[i] = strdup(g_env.envvar[i]);
    }
    g_env.old_envvar[i] = NULL;
}

static void free_env(char **env, int k)
{
    if (env != NULL)
    {
        for (; env[k]; k++)
        {   
            free(env[k]);
            env[k] = NULL;
        }
    }
}
/*
static char *var_name(char *arg)
{
    int n = strlen(arg);
    int i = 0;
    char *varname = xcalloc(n + 1, sizeof(char));
    while (arg[i] != '=')
    {
        varname[i] = arg[i];
        i++;
    }
    varname[i] = '\0'; 
    return varname;
}

//check if we got the right variable
static int is_right_var(char **var, char *arg, int i)
{
    if (strchr(arg, '=') == NULL)
    {
        char *name = var_name(var[i]);
        if (strcmp(name, arg) == 0)
        {
            free(name);
            return 0;
        }
        free(name);
        return 1;
    }
    else 
        return 0;                                
}
*/
static void simple_export(char **env)
{
    for(int i = 0; env[i] != NULL; i++)
        printf("%s\n", env[i]);
}

static void n_export(char *arg)
{
    int i = 0;
    int j = 0;
    int k = 0;
    free_env(g_env.envvar, k);

    while (strncmp(g_env.old_envvar[i], arg, strlen(arg)) != 0) 
        //&& (is_right_var(g_env.old_envvar, arg, i) != 0)) 
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


int export(char **argv)
{
    assert(argv && (strcmp(argv[0], "export") == 0));
    
    
    int argc = 0;
    while (argv[argc] != NULL)
        argc++;
    
    if (handle_error(argc, argv) == 1) //check if syntax error
        return 1;

    if (argc == 1 || (strcmp(argv[1], "-p") == 0))
        simple_export(g_env.envvar);

    if (argc == 3 && (strcmp(argv[1], "-n") == 0) && (strcmp(argv[2], "-p") == 0))
        simple_export(g_env.envvar);

    if (argc > 2 && (strcmp(argv[1], "-n")  == 0))
    {
        for (int i = 2; argv[i]; i++)
        {
            int k = 0;
            free_env(g_env.old_envvar, k);
            free(g_env.old_envvar);
            save_old_env();
            n_export(argv[i]);
            int j = 0;
            free_env(g_env.old_envvar, j);
            free(g_env.old_envvar);
            save_old_env();
        }
        return 0;
    }
    return 0;  
}
