#define POSIX_C_SOURCE 200809L
#include <criterion/criterion.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../../src/input_ouput/read.h"
#include "../../src/data_structures/data_string.h"

Test(read, basic_read)
{
    int fd = open("f_test", O_RDWR, 0644);
    struct string *str = init_string();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "Hello"));
    close(fd);
}

Test(read, basic_long_read)
{
    int fd = open("f_big_test", O_RDWR, 0644);
    struct string *str = init_string();
    xread(str, fd);
    cr_assert_eq(0, strcmp(str->content, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    close(fd);
}