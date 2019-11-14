#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "parser.h"
#include "../lexer/token_lexer.h"
#include "../memory/memory.h"
#include "../execution_handling/command_container.h"
#include "../data_structures/array_list.h"

static int next_is(struct queue *lexer, const char *to_match)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return 0;
    return strcmp(token->data, to_match) == 0;
}

//just need to free last token if there is one
static void skip_newline(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token != NULL && token->type == TOKEN_END_OF_LINE)
    {
        token_lexer_free(&token);
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

static struct instruction* parse_if(struct queue *lexer);

// à voir comment ça marche exactement
static struct instruction* parse_shell_command(struct queue *lexer)
{
    if (next_is(lexer, "if"))
        return parse_if(lexer);
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
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return true;
    return token->type == TOKEN_END_OF_INSTRUCTION
            || token->type == TOKEN_END_OF_LINE
            || token->type == TOKEN_OPERATOR;
}

static struct command_container* build_simple_command(char *simple_command,
                                                struct array_list *parameters
)
{
    return command_create(simple_command, parameters);
}

//simplified version of the grammar
static struct instruction* parse_simple_command(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_pop(lexer);
    char *simple_command = token->data;

    struct array_list *parameters = array_list_init();

    while (!next_is_end_of_instruction(lexer))
        array_list_append(parameters, token_lexer_pop(lexer));

    return build_instruction(TOKEN_COMMAND, build_simple_command(simple_command,
                                                                parameters));
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
    struct instruction *left = parse_pipeline(lexer);
    struct instruction *right = NULL;

    if (next_is(lexer, "||"))
    {
        token_lexer_pop(lexer);
        right = parse_and_or(lexer);
        return build_instruction(TOKEN_OR, build_and_or(left, right));
    }
    else if (next_is(lexer, "&&"))
    {
        token_lexer_pop(lexer);
        right = parse_and_or(lexer);
        return build_instruction(TOKEN_AND, build_and_or(left, right));
    }
    else
        return left;
}

//for now just call to and_or
static struct instruction* parse_compound_list_break(struct queue *lexer)
{
    return parse_and_or(lexer);
}

static struct if_instruction* build_if_instruction(
                                        struct instruction *conditions,
                                        struct instruction *to_execute,
                                        struct instruction *elif_container,
                                        struct instruction *else_container
)
{
    struct if_instruction *if_instruction = xmalloc(sizeof(*if_instruction));
    if_instruction->conditions = conditions;
    if_instruction->to_execute = to_execute;
    if_instruction->elif_container = elif_container;
    if_instruction->else_container = else_container;
    return if_instruction;
}

//pour l'instant gère pas les else et elif
static struct instruction* parse_if(struct queue *lexer)
{
    struct instruction *conditions = NULL;
    struct instruction *to_execute = NULL;
    struct instruction *elif_container = NULL;
    struct instruction *else_container = NULL;
    token_lexer_pop(lexer); //pop if
    conditions = parse_compound_list_break(lexer);
    token_lexer_pop(lexer); //pop ; ou \n
    token_lexer_pop(lexer); //pop then  to verify
    to_execute = parse_compound_list_break(lexer);
    token_lexer_pop(lexer); //pop fi
    return build_instruction(TOKEN_IF, build_if_instruction(conditions,
                                                        to_execute,
                                                        elif_container,
                                                        else_container));
}

//for now doens't handle comand;command
static struct instruction* parse_list(struct queue *lexer)
{
    struct instruction *and_or = parse_and_or(lexer);

    return and_or;
}

struct instruction* parse_input(struct queue *lexer)
{
    struct instruction *ast = parse_list(lexer);
    //skip_newline(lexer);
    return ast;
}