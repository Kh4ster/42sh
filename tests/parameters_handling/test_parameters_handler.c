#include <criterion/criterion.h>
#include <string.h>

#include "../../src/parameters_handling/options.h"
#include "../../src/parameters_handling/parameters_handler.h"

Test(arguments, no_arg)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh"};
    cr_assert_eq(1, handle_parameters(&options, 1, argv, 0), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_eq(0, options.option_o, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, arg_o)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-O"};
    cr_assert_eq(1, handle_parameters(&options, 2, argv, 0), "return val should be 1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_eq(1, options.option_o, "should be 1");
    cr_assert_null(options.command_option_c, "should be null");
}

Test(arguments, arg_c)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-c", "ls"};
    cr_assert_eq(1, handle_parameters(&options, 3, argv, 0), "return val should be 1");
    cr_assert_eq(1, options.option_c, "should be 1");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_eq(0, options.option_o, "should be 0");
    cr_assert_eq(0, strcmp(options.command_option_c, "ls"), "should be ls");
}

Test(arguments, arg_mix)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "-O", "-c", "ls", "--ast-print"};
    cr_assert_eq(1, handle_parameters(&options, 5, argv, 0), "return val should be 1");
    cr_assert_eq(1, options.option_c, "should be 1");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(1, options.option_a, "should be 1");
    cr_assert_eq(1, options.option_o, "should be 1");
    cr_assert_eq(0, strcmp(options.command_option_c, "ls"), "should be ls");
}

Test(arguments, error)
{
    struct boot_params options = {0};
    char *argv[] = {"42sh", "jioea"};
    cr_assert_eq(-1, handle_parameters(&options, 2, argv, 0), "return val should be -1");
    cr_assert_eq(0, options.option_c, "should be 0");
    cr_assert_eq(0, options.option_n, "should be 0");
    cr_assert_eq(0, options.option_a, "should be 0");
    cr_assert_eq(0, options.option_o, "should be 0");
    cr_assert_null(options.command_option_c, "should be null");
}