#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>

#include "alias.h"
#include "../../data_structures/hash_map.h"
#include "../../data_structures/array_list.h"
#include "../../input_output/get_next_line.h"

static void swap(void **a, void **b)
{
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}


static void sort_aliases(int nb_elements, void **aliases)
{
    char *res_local = setlocale(LC_ALL, "");

    if (!res_local)
    {
        warnx("unable to set locale");
        return;
    }

    for (int i = nb_elements - 1; i >= 0; i--)
    {
        for (int j = 0; j < i; j++)
        {
            char *first_alias = aliases[j];
            char *second_alias = aliases[j + 1];

            if (strcoll(first_alias, second_alias) > 0)
                swap(&(aliases[j]), &(aliases[j + 1]));
        }
    }
}

static void print_aliases(struct array_list *aliases)
{
    sort_aliases(aliases->nb_element, aliases->content);

    for (size_t i = 0; i < aliases->nb_element; i++)
    {
        char *to_print = aliases->content[i];
        printf("%s\n", to_print);
    }

    array_list_destroy(aliases);
}



static void create_and_print_aliases()
{
    struct hash_map *hash_m = g_env.aliases;
    struct array_list *aliases = array_list_init();

    for (size_t i = 0; i < hash_m->size; i++)
    {
        char *new;
        char *content = hash_m->slots[i].data;

        if (!content)
            continue;

        int error = asprintf(&new, "%s='%s'", hash_m->slots[i].key, content);

        if (error == -1)
        {
            array_list_destroy(aliases);
            warnx("Could not print aliases: error while creating the list");
            return;
        }

        array_list_append(aliases, strdup(new));
        free(new);
    }

    print_aliases(aliases);
}


static int print_alias(char *name, char *content)
{
    if (!content)
    {
        warnx("%s: alias not found", name);
        return 1;
    }

    printf("%s='%s'\n", name, content);
    return 0;
}


static int assign_or_print_alias(char *alias, int assign)
{
    char *has_equal = strpbrk(alias, "=");

    if (!has_equal)
    {
        char *alias_found = hash_find(g_env.aliases, alias);
        return print_alias(alias, alias_found);
    }

    else if (assign)
    {
        *has_equal = '\0';
        hash_insert(g_env.aliases, alias, has_equal + 1, STRING);
    }

    return 0;
}


int alias(char **args)
{
    if (!args[1])
    {
        create_and_print_aliases();
        return 0;
    }

    int has_to_assign = 1;
    size_t begin = 1;

    if (strcmp(args[1], "-p") == 0)
    {
        create_and_print_aliases();
        has_to_assign = 0;
        begin = 2;
    }


    int error = 0;

    for (; args[begin]; begin++)
    {
        int new_error = assign_or_print_alias(args[begin], has_to_assign);
        if (error == 0)
            error = new_error;
    }

    return error;
}
