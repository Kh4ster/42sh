#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command_storage.h"
#include "../memory/memory.h"
#include "command_execution.h"

int word_count(char *s)
{
    int counter = 0;
    for(int i = 0; s[i]; i++)
    {
        if(s[i] == ' ')
            counter++;
    }
    return counter + 1;
}


struct command_container *cmd_store(char *line)
{
    int n = word_count(line);
    struct command_container *save;
    save = xcalloc(1, sizeof(struct command_container));
    save->params = xcalloc(n + 1, sizeof(char *));
    save->command = xcalloc(2, sizeof(char *));
    char *tmp = xcalloc(1, sizeof(char *));
    tmp = strtok(line, " ");
    int i = 0;
    for (; line[i] != ' ' && line[i] != '\n' && line[i]; i++)
    {
        save->command[i] = line[i];
    }
    save->command[i] = 0;
    int j = 0;
    while (tmp)
    {
        save->params[j] = tmp;
        tmp = strtok(NULL, " ");
        j++;
    }
    /*
    for (int j = 0; line[j]; j++)
    {
        k = 0;
        while (line[j] != ' ')
        {
            tmp[l][k] = line[j];
            k++;
            j++;
        }
        tmp[k] = '\0';
        save->params[l] = tmp[l];
        l++;
        //str_init(tmp);
        }*/
    save->params[j] = NULL;
    free(tmp);
    return save;
}

int main()
{
    char *line = "echo toto papa";
    struct command_container *save = cmd_store(line);
    printf("%s\n", save->command);
    printf("%s\n", save->params[0]);
    //int exec = exec_cmd(save);
    free(save->command);
    command_destroy(&save);
    return 0;
}

