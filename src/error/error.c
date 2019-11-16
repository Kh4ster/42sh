#include <stdlib.h>
#include <err.h>

#include "../data_structures/queue.h"
#include "error.h"
#include "../lexer/token_lexer.h"

inline void handle_parser_errors(struct queue *lexer)
{
    if (g_env.is_parsing_ressource || is_interactive())
    {
        token_queue_empty(lexer);
        warnx("Bad parsing");
    }
    else
    {
        token_queue_free(&lexer);
        errx(2, "Bad parsing");
    }
}