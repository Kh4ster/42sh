#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "../lexer/token_lexer.h"
#include "../data_structures/queue.h"
#include "../memory/memory.h"
#include "../execution_handling/command_container.h"
#include "../data_structures/array_list.h"

static int next_is(struct queue *lexer, char *to_match)
{
    struct token_lexer *token = queue_head(lexer);
    if (token == NULL)
        return 0;
    return strcmp(token->data, to_match) == 0;
}

//just need to free last token if there is one
static void skip_newline(struct queue *lexer)
{
    struct token_lexer *token = queue_head(lexer);
    if (token != NULL && token->type == TOKEN_END_OF_LINE)
    {
        free_token(token);
    }
}

static struct instruction* build_instruction(enum token_parser_type type,
                                                            void *input_instr
)
{
    struct instruction *instruction = xmalloc(sizeof(*instruction));
    instruction->data = input_instr;
    instruction->type = type;
    return instruction;
}

static struct and_or_instruction* build_and_or(struct instruction *left,
                                    struct instruction *right
)
{
    struct and_or_instruction *and_or = xmalloc(sizeof(*and_or));
    and_or->left = left;
    and_or->right = right;
    return and_or;
}

//for now just call to and_or
static struct instruction* parse_compound_list_break(struct queue *lexer)
{
    return parse_and_or(lexer);
}

//pour l'instant gère pas les else et elif
static struct instruction* parse_if(struct queue *lexer)
{
    struct instruction *conditions = NULL;
    struct instruction *to_execute = NULL;
    struct instruction *elif_container = NULL;
    struct instruction *else_container = NULL;
    queue_pop(lexer); //pop if
    conditions = parse_compound_list_break(lexer);
    queue_pop(lexer); //pop then
    to_execute = parse_compound_list_break(lexer);
    queue_pop(lexer); //pop fi
}

// à voir comment ça marche exactement
static struct instruction* parse_shell_command(struct queue *lexer)
{
    if (next_is(lexer, "if"))
        return pase_if(lexer);
    assert(0 && "not yet implented");
    return NULL;
}

//missing other shell command
static bool is_shell_command(struct queue *lexer)
{
    static const char *builtins[] =
    {
        "if",
        "for"
    };
    for (size_t i = 0; i < sizeof(builtins) / sizeof(char*); ++i)
    {
        if(next_is(lexer, builtins[i]))
            return true;
    }
    return false;
}

static bool next_is_end_of_instruction(struct queue *lexer)
{
    struct token_lexer *token = queue_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_END_OF_INSTRUCTION
            || token->type == TOKEN_END_OF_LINE
            || token->type == TOKEN_OPERATOR;
}

static struct command_container* build_simple_command(struct queue *lexer)
{
    char *simple_command = queue_pop(lexer);
    struct array_list *parameters = init_array_list();
    while (!next_is_end_of_instruction(lexer))
        array_list_append(parameters, queue_pop(lexer));
    return command_container_init(simple_command, parameters);
}

//simplified version of the grammar
static struct instruction* parse_simple_command(struct queue *lexer)
{
    return build_instruction(TOKEN_COMMAND, build_simple_command(lexer));
}

//only handle shell command and simple command
static struct instruction* parse_command(struct queue *lexer)
{
    struct instruction *command = NULL;
    if (is_shell_command(lexer))
        command = parse_shell_command(lexer);
    else
        command = parse_simple_command(lexer);

    return command;
}

//doen't handle redirection
static struct instruction* parse_pipeline(struct queue *lexer)
{
    struct instruction *command = parse_command(lexer);

    return command;
}

static struct instruction* parse_and_or(struct queue *lexer)
{
    struct instruction *left = parse_pipline();
    struct instruction *right = NULL;

    if (next_is(lexer, "||"))
    {
        queue_pop(lexer);
        right = parse_and_or(lexer);
        return build_instruction(TOKEN_OR, build_and_or(left, right));
    }
    else if (next_is(lexer, "&&"))
    {
        queue_pop(lexer);
        right = parse_and_or(lexer);
        return build_instruction(TOKEN_AND, build_and_or(left, right));
    }
    else
        return left;
}

//for now doens't handle comand;command
static struct instruction* parse_instruction(struct queue *lexer)
{
    struct instruction *and_or = parse_and_or(lexer);

    return and_or;
}

struct instruction* parse_input(struct queue *lexer)
{
    struct instruction *ast = parse_instruction(lexer);
    skip_newline(lexer);
    return ast;
}