#include <err.h>

#include "../input_output/write.h"

int main(int argc, char *argv[])
{
    if (xwrite(argv[1], 1) == -1)
        errx(2, "Error echo");
}