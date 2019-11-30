#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <err.h>

#include "../memory/memory.h"
#include "export.h"
#include "../input_output/get_next_line.h"
#include "../data_structures/hash_map.h"

static int handle_error(int argc, char **arg)
{
    if (argc > 2 && arg[1] != NULL &&  (strcmp(arg[1], "-n") == 0)
        && (strcmp(arg[2], "-p") != 0))
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

static char *var_name(char *arg)
{
    int n = strlen(arg);
    int i = 0;
    char *varname = xcalloc(n + 1, sizeof(char));
    if (strchr(arg, '=') != NULL)
    {
        while (arg[i] != '=')
        {
            varname[i] = arg[i];
            i++;
        }
        varname[i] = '\0';
        return varname;
    }
    else
    {
        while (arg[i] != '\0')
        {
            varname[i] = arg[i];
            i++;
        }
        varname[i] = '\0';
        return varname;
    }
}

static char *var_value(char *arg)
{
    int n = strlen(arg);
    int i = 0;
    int j = 0;
    char *value = xcalloc(n + 1, sizeof(char));
    if (strchr(arg, '=') != NULL)
    {
        while (arg[i] != '=')
            i++;
        i++;
        while (arg[i])
        {
            value[j] = arg[i];
            i++;
            j++;
        }
        value[j] = '\0';
    }
    return value;
}

//check if we got the right variable
static int is_right_var(char **var, char *arg, int i)
{
    char *name1 = var_name(var[i]);
    char *name2 = var_name(arg);
    if (strcmp(name1, name2) == 0)
    {
        free(name1);
        free(name2);
        return 0;
    }
    free(name1);
    free(name2);
    return 1;
}

static void simple_export(char **env)
{
    char *value = NULL;
    char *name = NULL;
    for (int i = 0; env[i] != NULL; i++)
    {
        if (strchr(env[i], '=') != NULL)
        {
            name = var_name(env[i]);
            value = var_value(env[i]);
            printf("export %s=\"%s\"\n", name, value);
            free(name);
            free(value);
            name = NULL;
            value = NULL;
        }
        else
            printf("export %s\n", env[i]);
    }
}

static void export_n(char *arg)
{
    int i = 0;
    int j = 0;
    int k = 0;
    free_env(g_env.envvar, k);

    while (is_right_var(g_env.old_envvar, arg, i) != 0)
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

static void export_var(char *variable)
{
    char *save = strdup(variable);
    char *name = strtok_r(variable, "=", &variable);
    char *value = strtok_r(NULL, "=", &variable);
    hash_insert(g_env.variables, name, value, STRING);
    size_t i = 0;
    while (g_env.envvar[i] != NULL)
        i++;
    g_env.envvar = xrealloc(g_env.envvar, (i + 2) * sizeof(char *));
    g_env.envvar[i] = strdup(save);
    g_env.envvar[i + 1] = NULL;
    free(save);
}

static void change_value(char *var, int i)
{
    size_t n = strlen(var);
    char *value = var_value(var);
    char *name = var_name(g_env.envvar[i]);
    g_env.envvar[i] = xrealloc(g_env.envvar[i], (n + 1) * sizeof(char));
    name = xrealloc(name, (n + 1) * sizeof(char));
    strcat(name, "=");
    strcat(name, value);
    strcpy(g_env.envvar[i], name);
    free(name);
    free(value);
}

static int var_exists(char *var)
{
    for (int i = 0; g_env.envvar[i] != NULL; i++)
    {
        if ((strchr(g_env.envvar[i], '=') == NULL)
            && strchr(var, '=') == NULL)
        {
            if (strcmp(g_env.envvar[i], var) == 0)
            return 0;
        }
        if ((strchr(g_env.envvar[i], '=') != NULL)
            && strchr(var, '=') == NULL)
        {
            char *name = var_name(g_env.envvar[i]);
            if (strcmp(name, var) == 0)
            {
                free(name);
                return 0;
            }
            free(name);
            name = NULL;
        }
        if ((strchr(g_env.envvar[i], '=') == NULL)
            && strchr(var, '=') != NULL)
        {
            char *name = var_name(var);
            if (strcmp(g_env.envvar[i], name) == 0)
            {
                change_value(var, i);
                free(name);
                return 0;
            }
            free(name);
            name = NULL;
        }

        if ((strchr(g_env.envvar[i], '=') != NULL)
            && strchr(var, '=') != NULL)
        {
            if (strcmp(g_env.envvar[i], var) == 0)
            {
                return 0;
            }

            char *name1 = var_name(g_env.envvar[i]);
            char *name2 = var_name(var);
            if (strcmp(name1, name2) == 0)
            {
                change_value(var, i);
                free(name1);
                free(name2);
                return 0;
            }
            free(name1);
            free(name2);
            name1 = NULL;
            name2 = NULL;
        }
    }
    return 1;
}

//TODO : REFACTOR
int export(char **argv)
{
    assert(argv && (strcmp(argv[0], "export") == 0));

    int argc = 0;
    while (argv[argc] != NULL)
        argc++;

    if (handle_error(argc, argv) == 1) //check if syntax error
        return 1;

    if (argc == 1 || (strcmp(argv[1], "-p") == 0)
        || (argc == 2 && (strcmp(argv[1], "-n") == 0)))
        simple_export(g_env.envvar);

    else if (argc == 3 && (strcmp(argv[1], "-n") == 0)
             && (strcmp(argv[2], "-p") == 0))
        simple_export(g_env.envvar);

    else if (argc > 2 && (strcmp(argv[1], "-n")  == 0))
    {
        for (int i = 2; argv[i]; i++)
        {
            int k = 0;
            free_env(g_env.old_envvar, k);
            free(g_env.old_envvar);
            save_old_env();
            export_n(argv[i]);
            int j = 0;
            free_env(g_env.old_envvar, j);
            free(g_env.old_envvar);
            save_old_env();
        }
        return 0;
    }

    else //variable adding case
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '=')
            {
                fprintf(stderr, "export: '=' not a valid identifier\n");
                return 1;
            }
            if (var_exists(argv[i]) != 0) //var does not already exist
                export_var(argv[i]);
        }
    }
    return 0;
}