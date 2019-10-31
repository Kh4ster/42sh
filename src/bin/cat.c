#include <stdlib.h>
#include <stdio.h>

#include "../input_output/write.h"
#include "../input_output/read.h"
#include "../data_structures/data_string.h"

int main(void)
{
    struct string *buffer = string_init();
    xread(buffer, 0);
    string_append_char(buffer, '$');
    string_append_char(buffer, '\n');
    char *str_buffer = string_get_content(&buffer);
    xwrite(str_buffer, 1);
    free(str_buffer);
}