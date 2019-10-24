#define _POSIX_C_SOURCE 200809L
#include <criterion/criterion.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../../src/input_output/read.h"
#include "../../src/data_structures/data_string.h"

Test(read, basic_read)
{
    int fd = open("./bin/f_test", O_CREAT | O_EXCL | O_RDWR, 0644);
    dprintf(fd, "Hello");
    lseek(fd, 0, SEEK_SET);
    struct string *str = string_init();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "Hello"));
    string_free(&str);
    close(fd);
    remove("./bin/f_test");
}

Test(read, basic_long_read)
{
    int fd = open("./bin/f_test_big", O_CREAT | O_EXCL | O_RDWR, 0644);
    dprintf(fd, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    lseek(fd, 0, SEEK_SET);
    struct string *str = string_init();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    string_free(&str);
    close(fd);
    remove("./bin/f_test_big");
}