#include <criterion/criterion.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../../src/input_output/write.h"
#include "../../src/data_structures/data_string.h"
#include "../../src/input_output/read.h"

Test(write, basic_write)
{
    int fd = open("./f_test", O_CREAT | O_EXCL | O_RDWR, 0644);
    xwrite("Hello", fd);
    lseek(fd, 0, SEEK_SET);
    struct string *str = string_init();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "Hello"));
    string_free(&str);
    close(fd);
    remove("./f_test");
}

Test(write, basic_long_write)
{
    int fd = open("./f_test_big", O_CREAT | O_EXCL | O_RDWR, 0644);
    xwrite("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", fd);
    lseek(fd, 0, SEEK_SET);
    struct string *str = string_init();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    string_free(&str);
    close(fd);
    remove("./f_test_big");
}