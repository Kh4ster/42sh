#include <stdlib.h>
#include <err.h>

#include "../data_structures/queue.h"
#include "error.h"
#include "../lexer/token_lexer.h"

inline void handle_parser_errors(struct queue *lexer)
{
    token_queue_free(&lexer);
    err(2, "Bad parsing");
}