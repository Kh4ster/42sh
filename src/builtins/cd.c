#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "../input_output/get_next_line.h"
#include "../memory/memory.h"
#include "cd.h"

#define MAX_PATH_LENGTH 4096

void set_pwd(void)
{
    free(g_env.pwd);
    g_env.pwd = xmalloc(MAX_PATH_LENGTH);
    g_env.pwd = getcwd(g_env.pwd, MAX_PATH_LENGTH);
}

static char *xstrdup(char *str)
{
    char *copy_str = strdup(str);
    if (copy_str == NULL)
        memory_exhausted();
    return copy_str;
}

static void handle_error_chdir(int errno_chdir, char *path)
{
    if (errno_chdir == EACCES)
        fprintf(stderr, "cd: %s: Access denied.\n", path);
    else if (errno_chdir == ENAMETOOLONG)
        fprintf(stderr, "cd: %s: Path is too long.\n", path);
    else if (errno_chdir == ENOMEM)
        fprintf(stderr, "cd: %s: No such file or directory.\n", path);
    else if (errno_chdir == ENOTDIR)
        fprintf(stderr, "cd: %s: Not a directory.\n", path);
    else
        fprintf(stderr, "cd: %s\n", strerror(errno_chdir));
}

int my_chdir(char *dir)
{
    // check length of path given
    handle_error_chdir(strlen(dir) < MAX_PATH_LENGTH ? 0 : ENAMETOOLONG , dir);

    set_pwd();

    // absolute path
    if (dir[0] == '/')
    {
        if (chdir(dir) == -1)
        {
            handle_error_chdir(errno, dir);
            return 1;
        }
        return 0;
    }

    // relative path
    char *path = xmalloc(MAX_PATH_LENGTH);
    path[0] = '\0';
    if (strlen(g_env.pwd) + strlen(dir) > MAX_PATH_LENGTH)
    {
        fprintf(stderr, "cd: %s%s: Path is too long.\n", g_env.pwd, dir);
        return 1;
    }
    strcat(path, g_env.pwd);
    strcat(path, dir);

    if (chdir(path) == -1)
    {
        handle_error_chdir(errno, path);
        free(path);
        return 1;
    }
    free(g_env.pwd);
    g_env.pwd = xstrdup(path);
    free(path);
    return 0;
}

int cd(char **args)
{
    set_pwd();

    if (args[1] == NULL)
    {
        // go to HOME
        char *homedir = getenv("HOME");
        if (homedir == NULL)
            return 0;
        return my_chdir(homedir);
    }

    if (args[2] == NULL)
    {
        // go to dir given in args
        return my_chdir(args[2]);
    }

    // too many arguments
    fprintf(stderr, "cd: too many arguments");
    return 1;
}

int main()
{
    char *current_dir = xmalloc(MAX_PATH_LENGTH);
    current_dir = getcwd(current_dir, MAX_PATH_LENGTH);
    if (current_dir == NULL)
    {
        fprintf(stderr, "cd: error while searching for current dir.\n");
        return 1;
    }
    printf("current_dir : %s\n", current_dir);
    char *args[] = {"cd", NULL};
    cd(args);
    current_dir = getcwd(current_dir, 4096);
    printf("current_dir : %s\n", current_dir);
    char *args2[] = {"cd", "../memory", NULL};
    cd(args2);
    current_dir = getcwd(current_dir, 4096);
    printf("current_dir : %s\n", current_dir);
    free(current_dir);
}