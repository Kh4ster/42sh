#include <criterion/criterion.h>
#include <string.h>

#include "../../src/parameters_handling/options.h"
#include "../../src/parameters_handling/parameters_handler.h"
#include "../../src/input_output/get_next_line.h"

Test(arguments, no_arg)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh"};
    cr_assert_eq(1, handle_parameters(&options, 1, argv), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, arg_minus_o)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-O", NULL};
    cr_assert_eq(1, handle_parameters(&options, 2, argv), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, arg_plus_o)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "+O", NULL};
    cr_assert_eq(1, handle_parameters(&options, 2, argv), "return val should be -1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, arg_c)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-c", "ls"};
    cr_assert_eq(1, handle_parameters(&options, 3, argv), "return val should be 1");
    cr_assert_eq(1, options.option_c, "should be 1");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_eq(0, strcmp(options.command_option_c, "ls"), "should be ls");
}

Test(arguments, arg_mix)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-c", "ls", "--ast-print"};
    cr_assert_eq(1, handle_parameters(&options, 4, argv), "return val should be 1");
    cr_assert_eq(1, options.option_c, "should be 1");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(1, options.option_a, "should be 1");
    cr_assert_eq(0, strcmp(options.command_option_c, "ls"), "should be ls");
}

Test(arguments, arg_file_not_exec)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "not_executable"};
    cr_assert_eq(1, handle_parameters(&options, 2, argv), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, args_option_file_not_exec)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "--norc", "not_executable"};
    cr_assert_eq(1, handle_parameters(&options, 3, argv), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(1, options.option_n, "should be 1");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, error_bad_param)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-t"};
    cr_assert_eq(-1, handle_parameters(&options, 2, argv), "return val should be -1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, error_file_not_exist)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "jioea"};
    cr_assert_eq(-1, handle_parameters(&options, 2, argv), "return val should be -1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, option_pluso_bad_param)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "+O", "jio"};
    cr_assert_eq(-1, handle_parameters(&options, 3, argv), "return val should be -1");
}

Test(arguments, option_minuxo_good_param)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-O", "jio", NULL};
    cr_assert_eq(-1, handle_parameters(&options, 4, argv), "return val should be -1");
}

Test(arguments, option_minuso_good_param)
{
    char *argv[] = {"42sh", "-O", "nocaseglob", NULL};
    cr_assert_eq(false, g_env.options.option_nocaseglob);
    cr_assert_eq(1, handle_parameters(&(g_env.options), 3, argv), "return val should be 1");
    cr_assert_eq(true, g_env.options.option_nocaseglob);
    char *argv2[] = {"42sh", "+O", "nocaseglob", NULL};
    cr_assert_eq(1, handle_parameters(&(g_env.options), 3, argv2), "return val should be 1");
    cr_assert_eq(false, g_env.options.option_nocaseglob);
}

Test(arguments, option_minuso_good_param_and_file)
{
    char *argv[] = {"42sh", "-O", "nocaseglob", "not_executable", NULL};
    cr_assert_eq(false, g_env.options.option_nocaseglob);
    cr_assert_eq(1, handle_parameters(&(g_env.options), 4, argv), "return val should be 1");
    cr_assert_eq(true, g_env.options.option_nocaseglob);
}

Test(arguments, option_plus_good_param_and_file)
{
    char *argv[] = {"42sh", "+O", "nocaseglob", "not_executable", NULL};
    cr_assert_eq(false, g_env.options.option_nocaseglob);
    cr_assert_eq(1, handle_parameters(&(g_env.options), 4, argv), "return val should be 1");
    cr_assert_eq(false, g_env.options.option_nocaseglob);
}

Test(arguments, option_minuso_bad_param_and_file)
{
    char *argv[] = {"42sh", "-O", "jdisqo", "not_executable", NULL};
    cr_assert_eq(false, g_env.options.option_nocaseglob);
    cr_assert_eq(-1, handle_parameters(&(g_env.options), 4, argv), "return val should be -1");
}

Test(arguments, option_plus_bad_param_and_file)
{
    char *argv[] = {"42sh", "+O", "jidosq", "not_executable", NULL};
    cr_assert_eq(false, g_env.options.option_nocaseglob);
    cr_assert_eq(-1, handle_parameters(&(g_env.options), 4, argv), "return val should be -1");
}