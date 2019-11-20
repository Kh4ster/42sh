#include <criterion/criterion.h>
#include <stdbool.h>

#include "../../src/builtins/shopt.h"
#include "../../src/execution_handling/command_container.h"
#include "../../src/input_output/get_next_line.h"

Test(shopt, set_unset)
{
    struct command_container *container = command_init(2,
                                                    "shopt",
                                                    "-s",
                                                    "nullglob");
    cr_assert_eq(0, shopt(container->params));
    cr_assert_eq(true, g_env.options.option_nullglob);

    container = command_init(2, "shopt", "-u", "nullglob");
    cr_assert_eq(0, shopt(container->params));
    cr_assert_eq(false, g_env.options.option_nullglob);
}

Test(shopt, no_param)
{
    struct command_container *container = command_init(1,
                                                    "shopt",
                                                    "-s");
    cr_assert_eq(-1, shopt(container->params));
}

Test(shopt, bad_param)
{
    struct command_container *container = command_init(2,
                                                    "shopt",
                                                    "-rz",
                                                    "ast_print");
    cr_assert_eq(-1, shopt(container->params));
}