#include <stdio.h>

#include "export.h"

void simple_export()
{
    for(int i = 0; g_env.envvar[i] != NULL; i++)
        printf("%s\n", g_env.envvar[i]);
}

int export_builtin(char **argv)
{
    assert(argv && (strcmp(argv[0], "export")));

    int argc = 0;
    while (argv[argc] != NULL)
        argc++;

    if (argc == 1 || (argc == 2 && (strcmp(argv[1] == "-p") == 0)))
        simple_export(); 
    else
        return 0;    
}
