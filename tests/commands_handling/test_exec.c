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
    cr_assert_eq(0 ,exec_cmd(cmd));
    command_destroy(&cmd);
}

Test(cmd_exec, cmd_notfound_toto, .timeout = 0, .exit_code = 126)
{
    struct command_container *cmd = command_init(1, "cmd_notfound", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "cmd_notfound"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(127, exec_cmd(cmd));
    command_destroy(&cmd);
}

Test(cmd_exec, not_executable_toto, .timeout = 0, .exit_code = 126)
{
    struct command_container *cmd = command_init(1, "./not_executable", "toto");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "./not_executable"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(126, exec_cmd(cmd));
    command_destroy(&cmd);
}

Test(cmd_exec, ls_dir_notfound, .timeout = 0, .exit_code = 0)
{
    struct command_container *cmd = command_init(1, "ls", "dir_notfound");
    struct command_container *returned_val = command_init(1, "echo", "$?");
    cr_assert_not_null(cmd);
    cr_assert_not_null(returned_val);
    cr_assert_eq(0, strcmp(cmd->command, "ls"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    exec_cmd(cmd);
    int val = exec_cmd(returned_val);
    //cr_assert_eq(2, val);
    char tmp[10];
    sprintf(tmp, "%d", val);
    cr_assert_stdout_eq_str(tmp);
    command_destroy(&cmd);
    command_destroy(&returned_val);
}

Test(cmd_exec, tree_dir_notfound, .timeout = 0, .exit_code = 0)
{
    struct command_container *cmd = command_init(1, "tree", "dir_notfound");
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "tree"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    cr_assert_eq(0, exec_cmd(cmd));
    command_destroy(&cmd);
}

