#include <string.h>

#include "../data_structures/queue.h"
#include "token_lexer.h"

void skip_class(int (*classifier)(int c), char **cursor)
{
    while (classifier(**cursor))
        (*cursor)++;
}

struct queue *lexer(char *line)
{
    struct queue *token_queue = queue_init();

    char *cursor = line;

    while (cursor != '\0')
    {
//     skip_class(isblank, &line);
// find next_delimiter
    }

    return token_queue;
}