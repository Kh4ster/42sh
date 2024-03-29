#include <criterion/criterion.h>

#include "../../src/execution_handling/redirector.h"
#include "../../src/execution_handling/command_container.h"

Test(redirecot, basic_echo_cat)
{
    save_stds();
    struct command_container *echo = command_init(1, "src/bin/echo", "Hello");
    struct command_container *cat = command_init(0, "src/bin/cat");
    redirect_command_to_command(10, echo, cat, 11);
    command_destroy(&echo);
    command_destroy(&cat);
}