#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "get_next_line.h"
#include "shopt.h"

#define IS(X) strcmp(opt_name, X) == 0

static bool is_opt(char *opt_name)
{
    static const char *options =
    {
        "ast_print",
        "dotglob",
        "expand_aliases",
        "extglob",
        "nocaseglob",
        "nullglob",
        "sourcepath",
        "xpg_echo"
    };
    static const size_t size_array = sizeof(options) / sizeof(char*);

    for (size_t i = 0; i < size_array; ++i)
        if (strcmp(opt_name, options[i] == 0))
            return true;

    return false;
}

static int set_option(char *opt_name)
{
    if (!is_opt(opt_name))
        return -1;

    if (IS("ast_print"))
        g_env.options.option_a = true;
    else if (IS("dotglob"))
        g_env.options.option_dot_glob = true;
    else if (IS("expand_aliases"))
        g_env.options.option_expand_aliases = true;
    else if (IS("extglob"))
        g_env.options.option_extglob = true;
    else if (IS("nocaseglob"))
        g_env.options.option_nocaseglob = true;
    else if (IS("nullglob"))
        g_env.options.option_nullglob = true;
    else if (IS("sourcepath"))
        g_env.options.option_sourcepath = true;
    else if (IS("xpg_echo"))
        g_env.options.option_xpg_echo = true;

    return 1;
}

static int unset_option(char *opt_name)
{
    if (!is_opt(opt_name))
        return -1;

    if (IS("ast_print"))
        g_env.options.option_a = false;
    else if (IS("dotglob"))
        g_env.options.option_dot_glob = false;
    else if (IS("expand_aliases"))
        g_env.options.option_expand_aliases = false;
    else if (IS("extglob"))
        g_env.options.option_extglob = false;
    else if (IS("nocaseglob"))
        g_env.options.option_nocaseglob = false;
    else if (IS("nullglob"))
        g_env.options.option_nullglob = false;
    else if (IS("sourcepath"))
        g_env.options.option_sourcepath = false;
    else if (IS("xpg_echo"))
        g_env.options.option_xpg_echo = false;

    return 1;
}

int shopt(char *options[])
{
    char c;

    int argc = 0;
    while (options[argc] != NULL) //find how many argument in the *[]
        argc++;

    while (optind < argc)
    {
        c = getopt(argc, options, "s:u:");
        if (c  == -1)
            return -1;
        if (c == 's')
        {
            if (optarg == NULL)
                return -1;
            if (set_option(optarg) == -1)
                return -1;
        }
        else if (c == 'u')
        {
            if (optarg == NULL)
                return -1;
            if (unset_option(optarg) == -1)
                return -1;
        }
        else if (c == '?')
            return -1;
    }
    return 0;
}