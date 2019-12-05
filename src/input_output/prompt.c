#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "get_next_line.h"
#include "../data_structures/hash_map.h"
#include "../data_structures/data_string.h"
#include "../memory/memory.h"

static char *get_date(char *format)
{
    char *buffer = xcalloc(sizeof(char), 500);
    time_t t;
    struct tm *time_info;

    // get time
    time(&t);
    time_info = localtime(&t);

    strftime(buffer, 500, format, time_info);
    return buffer;
}

static int str_cmp_and_move(char **cursor, char *ref)
{
    // compare ref to beginning of cursor
    if (strncmp(*cursor, ref, strlen(ref)) == 0)
    {
        *cursor += strlen(ref); // if equal, move cursor
        return 1;
    }

    return 0;
}

static int handle_system_info(char **prompt_style,
        struct string *prompt_str)
{
    if (str_cmp_and_move(prompt_style, "\\h"))
    {
        char *hostname = xcalloc(sizeof(char), 500);
        gethostname(hostname, 500);
        char *next_point = strchr(hostname, '.');
        if (next_point != NULL)
            *next_point = '\0';
        string_append(prompt_str, hostname);
        free(hostname);
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\H"))
    {
        char *hostname = xcalloc(sizeof(char), 500);
        gethostname(hostname, 500);
        string_append(prompt_str, hostname);
        free(hostname);
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\u"))
    {
        char *username = getlogin();
        if (username != NULL)
            string_append(prompt_str, username);
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\w")
            || str_cmp_and_move(prompt_style, "\\W"))
    {
        char *pwd = get_current_dir_name();
        if (pwd != NULL)
        {
            string_append(prompt_str, pwd);
            free(pwd);
        }
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\$"))
    {
        string_append_char(prompt_str, geteuid() ? '$' : '#');
        return 1;
    }
    return 0;
}

static int handle_date(char **prompt_style,
        struct string *prompt_str)
{
    if (str_cmp_and_move(prompt_style, "\\d"))
    {
        char *date = get_date("%a %b %d");
        string_append(prompt_str, date);
        free(date);
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\D{"))
    {
        char *next_bracket = strchr(*prompt_style, '}');
        if (next_bracket == NULL)
        {
            string_append(prompt_str, *prompt_style);
            *prompt_style += strlen(*prompt_style);
        }
        else
        {
            *next_bracket = '\0';
            char *format = strdup(*prompt_style);
            char *date = get_date(format);
            string_append(prompt_str, date);
            free(format);
            free(date);
            *next_bracket = '}';
            *prompt_style = next_bracket + 1;
        }
        return 1;
    }
    return 0;
}

static int handle_simple_escape(char **prompt_style,
        struct string *prompt_str)
{
    if (str_cmp_and_move(prompt_style, "\\a"))
    {
        string_append_char(prompt_str, '\a');
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\e"))
    {
        string_append_char(prompt_str, 27);
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\n"))
    {
        string_append_char(prompt_str, '\n');
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\r"))
    {
        string_append_char(prompt_str, '\r');
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\s"))
    {
        string_append(prompt_str, "42sh");
        return 1;
    }
    else if (str_cmp_and_move(prompt_style, "\\"))
    {
        string_append_char(prompt_str, '\\');
        return 1;
    }
    return 0;
}

static char *convert_prompt(char *prompt_style)
{
    struct string *prompt_str = string_init();

    while (*prompt_style != '\0')
    {
        if (handle_system_info(&prompt_style, prompt_str))
            continue;
        else if (handle_date(&prompt_style, prompt_str))
            continue;
        else if (handle_simple_escape(&prompt_style, prompt_str))
            continue;
        else
        {
            string_append_char(prompt_str, *prompt_style);
            prompt_style++;
        }
    }

    return string_get_content(&prompt_str);
}

char *get_prompt(int prompt)
{
    if (prompt == 0)
        return NULL;

    char *prompt_variable;
    asprintf(&prompt_variable, "PS%d", prompt);
    char *prompt_str = convert_prompt(hash_find(g_env.variables,
                                                prompt_variable));
    free(prompt_variable);
    return prompt_str;
}