#include <err.h>

#include "../input_output/write.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    argc++;
    if (xwrite(argv[1], 1) == -1)
        return -1;
    return 0;
}