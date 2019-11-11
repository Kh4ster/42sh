#include <criterion/criterion.h>
#include <stdlib.h>

#include "../../src/execution_handling/redirector.h"
#include "../../src/execution_handling/command_container.h"
#include "../../src/execution_handling/command_execution.h"
#include "../../src/memory/memory.h"

Test(cmd_exec, echo toto)
{
    struct command_container *cmd = command_init(1, "echo", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "echo"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(0, exec);
    free(cmd);
}

Test(cmd_exec, cmd_notfound toto)
{
    struct command_container *cmd = command_init(1, "cmd_notfound", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "cmd_notfound"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(127, exec);
    free(cmd);
}

Test(cmd_exec, ./not_executable toto)
{
    struct command_container *cmd = command_init(1, "./not_executable", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "./not_executable"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(126, exec);
    free(cmd);
}

Test(cmd_exec, ls dir_notfound)
{
    struct command_container *cmd = command_init(1, "ls", "dir_notfound");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "ls"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(2, exec);
    free(cmd);
}

Test(cmd_exec, tree dir_notfound)
{
    struct command_container *cmd = command_init(1, "tree", "dir_notfound");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "tree"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(0, exec);
    free(cmd);
}
