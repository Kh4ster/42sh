#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "parser.h"
#include "ast/ast.h"
#include "ast/destroy_tree.h"
#include "../lexer/token_lexer.h"
#include "../memory/memory.h"
#include "../execution_handling/command_container.h"
#include "../data_structures/array_list.h"

#define NEXT_IS(X) next_is(lexer, X)
#define EAT() eat_token(lexer)
#define NEXT_IS_EOF() next_is_eof(lexer)

//free all instructions passed and returns NULL
static void* free_instructions(size_t nb_param, ...)
{
    va_list ap;
    va_start(ap, nb_param);

    for (size_t i = 0; i < nb_param; ++i)
    {
        struct instruction *to_free = va_arg(ap, struct instruction*);
        destroy_tree(to_free);
    }

    va_end(ap);

    return NULL;
}

static bool next_is_eof(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_EOF;
}

static void eat_token(struct queue *lexer)
{
    struct token_lexer *to_free = token_lexer_pop(lexer);
    token_lexer_free(&to_free);
}

static bool next_is(struct queue *lexer, const char *to_match)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return strcmp(token->data, to_match) == 0;
}

static struct instruction* build_instruction(enum token_parser_type type,
                                                            void *input_instr
)
{
    struct instruction *instruction = xmalloc(sizeof(*instruction));
    instruction->data = input_instr;
    instruction->type = type;
    instruction->next = NULL;
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

// see how it works exactly
static struct instruction* parse_shell_command(struct queue *lexer)
{
    if (NEXT_IS("if"))
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
            || token->type == TOKEN_EOF
            || token->type == TOKEN_OPERATOR;
}

static struct command_container* build_simple_command(char *simple_command,
                                                struct array_list *parameters
)
{
    return command_create(simple_command, parameters);
}

//simplified version of the grammar
//doesn't handle redir
//here doesn't call eat cause we need the token data
static struct instruction* parse_simple_command(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_pop(lexer);
    free(token);
    char *simple_command_str = token->data;

    struct array_list *parameters = array_list_init();

    while (!next_is_end_of_instruction(lexer))
    {
        token = token_lexer_pop(lexer);
        free(token);
        array_list_append(parameters, token->data);
    }

    struct instruction *command = build_instruction(TOKEN_COMMAND,
                                                    build_simple_command(
                                                            simple_command_str,
                                                            parameters));

    free(parameters);//not destroy array_list cause we need it's content
    return command;

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

//not exactly grammar
static struct instruction* parse_pipeline(struct queue *lexer)
{
    struct instruction *command = parse_command(lexer);

    return command;
}

//not exactly grammar
//for now handle the * recursively
static struct instruction* parse_and_or(struct queue *lexer)
{
    struct instruction *left = NULL;
    if ((left = parse_pipeline(lexer)) == NULL)
        return NULL;

    struct instruction *right = NULL;

    if (NEXT_IS("||") || NEXT_IS("&&"))
    {
        enum token_parser_type type;
        if (NEXT_IS("||"))
            type = TOKEN_OR;
        else
            type = TOKEN_AND;
        EAT();

        while (NEXT_IS("\n"))
            EAT();

        if ((right = parse_and_or(lexer)) == NULL)
            return free_instructions(1, left);

        return build_instruction(type, build_and_or(left, right));
    }
    else
        return left;
}

//still not exactly grammar
//maybe refactor while part (also in parse list)
static struct instruction* parse_compound_list_break(struct queue *lexer)
{
    while (NEXT_IS("\n"))
        EAT();

    struct instruction *and_or = NULL;

    if ((and_or = parse_and_or(lexer)) == NULL)
        return NULL;

    if (NEXT_IS(";") || NEXT_IS("\n") || NEXT_IS("&"))
    {
        while (NEXT_IS("\n"))
            EAT();

        struct instruction *tmp_ast = and_or;

        while (NEXT_IS(";") || NEXT_IS("\n") || NEXT_IS("&"))
        {
            EAT();
            if ((tmp_ast->next = parse_and_or(lexer)) == NULL)
                return free_instructions(1, and_or);
            tmp_ast = tmp_ast->next;
        }
    }

    while (NEXT_IS("\n"))
        EAT();

    return and_or;
}

static struct if_instruction* build_if_instruction(
                                        struct instruction *conditions,
                                        struct instruction *to_execute,
                                        struct instruction *else_container
)
{
    struct if_instruction *if_instruction = xmalloc(sizeof(*if_instruction));
    if_instruction->conditions = conditions;
    if_instruction->to_execute = to_execute;
    if_instruction->else_container = else_container;
    return if_instruction;
}

static struct instruction* parse_else_clause(struct queue *lexer)
{
    if (NEXT_IS("else"))
    {
        EAT();
        return parse_compound_list_break(lexer);
    }
    else
    {
        struct instruction *conditions = NULL;
        struct instruction *to_execute = NULL;
        struct instruction *another_else = NULL;

        conditions = parse_compound_list_break(lexer);

        if (conditions == NULL || !NEXT_IS("then"))
            return free_instructions(1, conditions);
        EAT();

        if ((to_execute = parse_compound_list_break(lexer)) == NULL)
            return free_instructions(1, conditions);

        if(NEXT_IS("else") || NEXT_IS("elif"))
        {
            if ((another_else = parse_else_clause(lexer)) == NULL)
                return free_instructions(2, conditions, to_execute);
        }

        return build_instruction(TOKEN_IF, build_if_instruction(conditions,
                                                        to_execute,
                                                        another_else));
    }
}

static struct instruction* parse_if(struct queue *lexer)
{
    struct instruction *conditions = NULL;
    struct instruction *to_execute = NULL;
    struct instruction *else_container = NULL;

    if (!NEXT_IS("if"))
        return NULL;
    EAT();

    conditions = parse_compound_list_break(lexer);

    if (conditions == NULL || !NEXT_IS("then"))
        return free_instructions(1, conditions);
    EAT();

    if ((to_execute = parse_compound_list_break(lexer)) == NULL)
        return free_instructions(1, conditions);

    if(NEXT_IS("else") || NEXT_IS("elif"))
    {
        else_container = parse_else_clause(lexer);
        if (else_container == NULL)
            return free_instructions(2, conditions, to_execute);
    }

    if (!NEXT_IS("fi"))
        return free_instructions(3, conditions, to_execute, else_container);
    EAT();

    return build_instruction(TOKEN_IF, build_if_instruction(conditions,
                                                        to_execute,
                                                        else_container));
}

//for now doens't handle comand;command
static struct instruction* parse_list(struct queue *lexer)
{
    struct instruction *and_or = parse_and_or(lexer);

    return and_or;
}

//return null if only \n ?
//for now doens't handle if end with ; or with &
struct instruction* parse_input(struct queue *lexer)
{
    if (NEXT_IS("\n") || NEXT_IS_EOF())
        return NULL;

    struct instruction *ast = NULL;

    if ((ast = parse_list(lexer)) == NULL)
        return NULL;

    struct instruction *tmp_ast = ast;

    while (NEXT_IS(";") || NEXT_IS("&"))
    {
        EAT();
        if ((tmp_ast->next = parse_and_or(lexer)) == NULL)
            return free_instructions(1, ast);//if error we need to free all ast
        tmp_ast = tmp_ast->next;
    }

    if (!NEXT_IS("\n") && !NEXT_IS_EOF())
    {
        destroy_tree(ast);
        return NULL;
    }
    return ast;
}