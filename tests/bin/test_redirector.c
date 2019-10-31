#include <criterion/criterion.h>

#include "../../src/execution_handling/redirector.h"
#include "../../src/execution_handling/command_container.h"

/*
Test(redirecot, basic_echo_cat)
{
    save_stds();
    struct command_container *echo = command_init(1, "echo", "Hello");
    struct command_container *cat = command_init(0, "cat");
    redirect_command_to_command(0, echo, cat, 1);
}
*/
int main(void)
{
    save_stds();
    struct command_container *echo = command_init(1, "/home/khaster/42sh/src/bin/echo", "Hello");
    struct command_container *cat = command_init(0, "/home/khaster/42sh/src/bin/cat");
    redirect_command_to_command(10, echo, cat, 11);
}