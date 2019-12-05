#include <stdio.h>

#include "parser.h"
#include "tree.h"

int main(int argc, char **argv)
{
    if (argc == 0)
        return 0;

    struct node *root = parser(argv[1]);
    printf("result = %d\n", evaluate_tree(root));
}
