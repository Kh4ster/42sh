#include "../data_structures/queue.h"
#include "token_lexer.h"

struct queue *lexer(char *line)
{
    struct queue *new_queue = queue_init();
    return new_queue;
}