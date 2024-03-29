#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdio.h>

#include "../input_output/get_next_line.h"
#include "shopt.h"

#define IS(X) strcmp(opt_name, X) == 0

static bool is_shopt_var(char *opt_name)
{
    static const char *options[] =
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
    static const size_t size_array = sizeof(options) / sizeof(char *);

    for (size_t i = 0; i < size_array; ++i)
        if (strcmp(opt_name, options[i]) == 0)
            return true;

    return false;
}

static void set_option(char *opt_name)
{
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
}

static void unset_option(char *opt_name)
{
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
}

static bool return_shopt_value(char *opt_name)
{
    if (IS("ast_print"))
        return g_env.options.option_a;
    else if (IS("dotglob"))
        return g_env.options.option_dot_glob;
    else if (IS("expand_aliases"))
        return g_env.options.option_expand_aliases;
    else if (IS("extglob"))
        return g_env.options.option_extglob;
    else if (IS("nocaseglob"))
        return g_env.options.option_nocaseglob;
    else if (IS("nullglob"))
        return g_env.options.option_nullglob;
    else if (IS("sourcepath"))
        return g_env.options.option_sourcepath;
    else if (IS("xpg_echo"))
        return g_env.options.option_xpg_echo;
    return false; //impossible
}

static int width(const char *option)
{
    int n = strlen(option);
    int mod = 7 - (n % 8);
    if (n < 8)
    {
        mod += n % 8;
        if (n == 6)
            mod++;
        if (n == 7)
            mod++;
    }
    if (n == 14)
        mod = 1;
    return mod;
}

static int option_list(void)
{
    printf("ast_print%*s\t%s\n", width("ast_print"), ""
            , (g_env.options.option_a) ? "on" : "off");
    printf("dotglob%*s\t%s\n", width("dotglob"), ""
            , (g_env.options.option_dot_glob) ? "on" : "off");
    printf("expand_aliases%*s\t%s\n", width("expand_aliases")
            , "", (g_env.options.option_expand_aliases) ? "on" : "off");
    printf("extglob%*s\t%s\n", width("extglob"), ""
            , (g_env.options.option_extglob) ? "on" : "off");
    printf("nocaseglob%*s\t%s\n", width("nocaseglob"), ""
            , (g_env.options.option_nocaseglob) ? "on" : "off");
    printf("nullglob%*s\t%s\n", width("nullglob"), ""
            , (g_env.options.option_nullglob) ? "on" : "off");
    printf("sourcepath%*s\t%s\n", width("sourcepath"), ""
            , (g_env.options.option_sourcepath) ? "on" : "off");
    printf("xpg_echo%*s\t%s\n", width("xpg_echo"), ""
            , (g_env.options.option_xpg_echo) ? "on" : "off");
    return 0;
}

static int handle_option_no_var(char *opt_name)
{
    if (IS("-q"))
        return 0;

    bool on;
    if (IS("-s"))
        on = true;
    else
        on = false;

    static const char *options[] =
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
    static const size_t size_array = sizeof(options) / sizeof(char *);

    bool val_options[] =
    {
        g_env.options.option_a,
        g_env.options.option_dot_glob,
        g_env.options.option_expand_aliases,
        g_env.options.option_extglob,
        g_env.options.option_nocaseglob,
        g_env.options.option_nullglob,
        g_env.options.option_sourcepath,
        g_env.options.option_xpg_echo
    };

    for (size_t i = 0; i < size_array; ++i)
    {
        if ((on) ? val_options[i] : !val_options[i])
        {
            printf("%s%*s\t%s\n", options[i], width(options[i]), "",
                                                        (on) ? "on" : "off");
        }
    }
    return 0;
}

static bool check_options(char *option)
{
    return strcmp("-u", option) ==  0 || strcmp("-s", option) == 0
                                                || strcmp("-q", option) == 0;
}

int shopt(char *options[])
{
    int argc = 0;

    while (options[argc] != NULL) //find how many argument in the *[]
        argc++;

    if (argc == 1) //case call to shopt only
        return option_list();


    if (!check_options(options[1]) && (strchr(options[1], '-') != NULL))
    {
        warnx("Bad shopt option");
        return 2;
    }

    if ((options[2] == NULL) && ((strcmp("-s", options[1]) == 0)
        || (strcmp("-u", options[1]) == 0))) //case call with just -u -s
        return handle_option_no_var(options[1]);

    int to_return = 0;
    if (strcmp("-s", options[1]) == 0) // call with parameter
    {
        for (size_t i = 2; options[i] != NULL; ++i)
        {
            if (!is_shopt_var(options[i]))
            {
                warnx("%s: invalid shopt variable name", options[i]);
                to_return = 1;
            }
            else
                set_option(options[i]);
        }
    }
    else if (strcmp("-u", options[1]) == 0)
    {
        for (size_t i = 2; options[i] != NULL; ++i)
        {
            if (!is_shopt_var(options[i]))
            {
                warnx("%s: invalid shopt variable name", options[i]);
                to_return = 1;
            }
            else
                unset_option(options[i]);
        }
    }
    else if (strcmp("-q", options[1]) == 0) //multiple options ?
    {
        for (size_t i = 2; options[i] != NULL; ++i)
        {
            if (!is_shopt_var(options[i]))
            {
                warnx("%s: invalid shopt variable name", options[i]);
                return 1;
            }
            if (!return_shopt_value(options[i]))
                return 1;
        }
    }
    else //case shopt call with not option but shopt var
    {
        for (int i = 1; i < argc; i++)
        {
            if (is_shopt_var(options[i])) //case call to shopt and valid variable
            {
                bool local_return;
                if (!(local_return = return_shopt_value(options[i])))
                    to_return = 1;
                printf("%s%*s\t%s\n", options[i], width(options[i]), "",
                        (local_return) ? "on" : "off");
            }
            else
            {
                warnx("Option not in shopt");
                to_return = 1;
            }
        }
    }
    return to_return;
}