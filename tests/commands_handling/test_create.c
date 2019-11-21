#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../src/execution_handling/redirector.h"
#include "../../src/execution_handling/command_container.h"
#include "../../src/execution_handling/command_execution.h"
#include "../../src/memory/memory.h"
#include "../../src/data_structures/array_list.h"

Test(cmd_exec, echo_toto, .timeout = 0)
{
    char *c = strdup("echo");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup("toto"));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "echo"));
    cr_assert_eq(0, strcmp(cmd->params[0], "echo"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(0,exec_cmd(cmd));
    command_destroy(&cmd);
    array_list_destroy(list);
}

Test(cmd_exec, cmd_not_found_toto, .timeout = 0)
{
    char *c = strdup("cmd_notfound");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup("toto"));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "cmd_notfound"));
    cr_assert_eq(0, strcmp(cmd->params[0], "cmd_notfound"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(127,exec_cmd(cmd));
    command_destroy(&cmd);
    array_list_destroy(list);
}

Test(cmd_exec, not_executable_toto, .timeout = 0)
{
    FILE *fd;
    fd = fopen("not_executable", "w+");
    char *c = strdup("./not_executable");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup("toto"));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "./not_executable"));
    cr_assert_eq(0, strcmp(cmd->params[0], "./not_executable"));
    cr_assert_eq(0, strcmp(cmd->params[1], "toto"));
    cr_assert_eq(126,exec_cmd(cmd));
    fclose(fd);
    command_destroy(&cmd);
    array_list_destroy(list);
}

Test(cmd_exec, ls_dir_notfound, .timeout = 0)
{
    char *c = strdup("ls");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup("dir_notfound"));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "ls"));
    cr_assert_eq(0, strcmp(cmd->params[0], "ls"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    cr_assert_eq(2,exec_cmd(cmd));
    command_destroy(&cmd);
    array_list_destroy(list);
}

Test(cmd_exec, ls_prevdir_rootdir, .timeout = 0)
{
    char *c = strdup("ls");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup(".."));
    array_list_append(list, strdup("."));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "ls"));
    cr_assert_eq(0, strcmp(cmd->params[0], "ls"));
    cr_assert_eq(0, strcmp(cmd->params[1], ".."));
    cr_assert_eq(0, strcmp(cmd->params[2], "."));
    cr_assert_eq(0, exec_cmd(cmd));
    command_destroy(&cmd);
    array_list_destroy(list);
}

int main()
{
    char *c = strdup("tree");
    struct array_list *list = array_list_init();
    array_list_append(list, strdup("dir_notfound"));
    struct command_container *cmd = command_create(c, list);
    cr_assert_not_null(cmd);
    cr_assert_eq(0, strcmp(cmd->command, "tree"));
    cr_assert_eq(0, strcmp(cmd->params[0], "tree"));
    cr_assert_eq(0, strcmp(cmd->params[1], "dir_notfound"));
    cr_assert_eq(0, exec_cmd(cmd));
    command_destroy(&cmd);
    free(list->content);
    free(list);
}