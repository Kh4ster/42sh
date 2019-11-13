#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../src/execution_handling/redirector.h"
#include "../../src/execution_handling/command_container.h"
#include "../../src/execution_handling/command_execution.h"
#include "../../src/memory/memory.h"

Test(cmd_exec, echo_toto, .exit_code = 0)
{
    struct command_container *cmd = command_init(1, "echo", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "echo"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(0,exec_cmd(cmd));
    command_destroy(&cmd);
}

Test(cmd_exec, cmd_notfound_toto, .timeout = 0)
{
    struct command_container *cmd = command_init(1, "cmd_notfound", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "cmd_notfound"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(127, exec_cmd(cmd));
    command_destroy(&cmd);
}

Test(cmd_exec, not_executable_toto, .timeout = 0)
{
    FILE *fd;
    fd = fopen("not_executable", "w+");
    struct command_container *cmd = command_init(1, "./not_executable", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "./not_executable"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(126, exec_cmd(cmd));
    fclose(fd);
    command_destroy(&cmd);
}

Test(cmd_exec, ls_dir_notfound, .timeout = 0)
{
    struct command_container *cmd = command_init(1, "ls", "dir_notfound");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "ls"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    int exec = exec_cmd(cmd);
    cr_assert_eq(2, exec);
    command_destroy(&cmd);
}

Test(cmd_exec, tree_dir_notfound, .timeout = 0)
{
    struct command_container *cmd = command_init(1, "tree", "dir_notfound");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "tree"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    cr_assert_eq(0, exec_cmd(cmd));
    command_destroy(&cmd);
}

