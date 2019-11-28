#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

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
        g_env.old_envvar[i] = g_env.envvar[i];
    }
    g_env.old_envvar[i] = '\0';
}

static void n_export(char *arg)
{
    if (getenv(arg) == NULL)
        return;
    
    int size = 0;
    while(g_env.envvar[size])
        size++;

    int i = 0;
    int j = 0;
    while (strstr(g_env.old_envvar[i], arg) != NULL)  
    {
        g_env.envvar[j] = g_env.old_envvar[i];
        i++;
        j++;        
    }
    i++;
    while (g_env.old_envvar[i] != NULL)  
    {
        g_env.envvar[j] = g_env.old_envvar[i];
        i++;
        j++;        
    }

}
/*
static int handle_error(char *arg)
{
    return 1;
}
*/
int export(char **argv)
{
    assert(argv && (strcmp(argv[0], "export")));

    int argc = 0;
    while (argv[argc] != NULL)
        argc++;

    //save_old_env();
    if (argc == 1  || (argc == 2 && (strcmp(argv[1], "-p") == 0)))
        simple_export(g_env.envvar);

    if (argc > 2 && (strcmp(argv[1], "-n")  == 0) && argv[2])
    {
        int i = 2;
        while (argv[i])
        {
            //handle_error(argv[2]); //handle syntax error
            n_export(argv[i]);
            save_old_env();
            i++;
        }
        return 0;
    }

    return 0;  

}
