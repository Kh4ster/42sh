#include <fnmatch.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>

#include "../input_output/get_next_line.h"
#include "path_exepension.h"
#include "../data_structures/array_list.h"
#include "../memory/memory.h"

static int is_path_expansion(char *pattern)
{
    assert(pattern);

    for (size_t i = 0; pattern[i]; i++)
    {
        if (pattern[i] == '*')
            return 1;

        if (pattern[i] == '?')
            return 1;

        if (pattern[i] == '[')
            return 1;
    }

    return 0;
}


static int get_fnmatch_flags(void)
{
    int flags = FNM_PATHNAME;

    if (g_env.options.option_nocaseglob)
        flags = flags | FNM_CASEFOLD;

    if (g_env.options.option_extglob)
        flags = flags | FNM_EXTMATCH;

    return flags;
}


static int correct_file_name(char *file_name)
{
    if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
        return 0;

    if (*file_name == '.' && !g_env.options.option_dot_glob)
        return 0;

    return 1;
}


static char *create_path(char *path, char *file_name)
{
    if (strlen(path) == 0)
        return strdup(file_name);

    char *new_path = NULL;
    int ouai;

    if (path[strlen(path) - 1] != '/')
    {
        ouai = asprintf(&new_path, "%s/%s", path, file_name);
    }
    else
    {
        ouai = asprintf(&new_path, "%s%s", path, file_name);
    }

    if (ouai != -1)
        return new_path;

    else
        return NULL;
}


static void add_string_to_glob(struct path_globbing *path_glob, char *s)
{
    array_list_append(path_glob->matches, strdup(s));
    path_glob->nb_matchs++;
}

static int match_filesnames(char *path, DIR *current_dir,
                              struct path_globbing *path_glob, char *pattern)
{
    struct dirent *current_file = readdir(current_dir);
    char *tmp = NULL;
    int value = 0;

    for (; current_file; current_file = readdir(current_dir))
    {
        if (!correct_file_name(current_file->d_name))
            continue;

        tmp = create_path(path, current_file->d_name);

        if (fnmatch(pattern, tmp, get_fnmatch_flags()) == 0)
            add_string_to_glob(path_glob, tmp);

        if (current_file->d_type == DT_DIR)
        {
            DIR *new_dir = opendir(tmp);

            if (new_dir)
            {
                value = match_filesnames(tmp, new_dir, path_glob, pattern);
                closedir(new_dir);
            }
        }

        free(tmp);
    }

    return value;
}



static struct path_globbing *init_path_glob(void)
{
    struct path_globbing *p_glob = xmalloc(sizeof(struct path_globbing));
    p_glob->nb_matchs = 0;
    p_glob->matches = array_list_init();
    return p_glob;
}


static void swap(void **a, void **b)
{
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}


static void sort_matches(struct array_list *list)
{
    for (int i = list->nb_element - 1; i >= 0; i--)
    {
        for (int j = 0; j < i; j++)
        {
            if (strcoll(list->content[j], list->content[j + 1]) > 0)
                swap(&(list->content[j]), &(list->content[j + 1]));
        }
    }
}

extern struct path_globbing *sh_glob(char *pattern)
{
    if (!is_path_expansion(pattern))
        return NULL;

    DIR *current_dir_d = opendir(pattern);
    char *current_dir = pattern;

    if (!current_dir_d)
    {
        current_dir = get_current_dir_name();
        current_dir_d = opendir(current_dir);
    }

    if (!current_dir_d)
    {
        warn("Could not open directory %s", current_dir);
        free(current_dir);
        return NULL;
    }

    struct path_globbing *p_glob = init_path_glob();

    int error = match_filesnames("", current_dir_d, p_glob, pattern);

    free(current_dir);
    closedir(current_dir_d);

    if (error || (p_glob->nb_matchs == 0 && !g_env.options.option_nullglob))
    {
        destroy_path_glob(p_glob);
        return NULL;
    }
    sort_matches(p_glob->matches);
    return p_glob;
}

extern void destroy_path_glob(struct path_globbing *p_glob)
{
    array_list_destroy(p_glob->matches);
    free(p_glob);
}
