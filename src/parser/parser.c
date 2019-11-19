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
#define NEXT_IS_OTHER() next_is_other(lexer)
#define NEXT_IS_ASSIGNEMENT() next_is_assignement(lexer)
#define NEXT_IS_NUMBER() next_is_number(lexer)

static bool next_is_assignement(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_ASSIGNEMENT;
}


static bool next_is_other(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return false;
    return token->type == TOKEN_OTHER;
}


static bool next_is_number(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);

    if (token == NULL)
        return false;

    return token->type == TOKEN_IO_NUMBER;
}


// free all instructions passed and returns NULL
static void *free_instructions(size_t nb_param, ...)
{
    va_list ap;
    va_start(ap, nb_param);

    for (size_t i = 0; i < nb_param; ++i)
    {
        struct instruction *to_free = va_arg(ap, struct instruction *);
        destroy_tree(to_free);
    }

    va_end(ap);

    return NULL;
}

static enum token_parser_type token_is_redirection (struct token_lexer *token)
{
    enum token_parser_type type = 0;

    if (!strcmp(token->data, ">"))
        type = TOKEN_REDIRECT_LEFT;

    if (!strcmp(token->data, "<"))
        type = TOKEN_REDIRECT_RIGHT;

    if (!strcmp(token->data, ">>"))
        type = TOKEN_REDIRECT_APPEND_LEFT;

    return type;

}

static enum token_parser_type is_redirection(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);

    return token_is_redirection(token);
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

static struct redirection *build_redirection(int fd, char *file)
{
    struct redirection *redirect = xmalloc(sizeof(struct redirection));
    redirect->to_redirect = NULL;
    redirect->fd = fd;
    redirect->file = file;
    return redirect;
}


struct instruction *build_instruction(enum token_parser_type type,
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

static struct instruction *parse_if(struct queue *lexer);
static struct instruction *parse_while_clause(struct queue *lexer);

// see how it works exactly
static struct instruction *parse_shell_command(struct queue *lexer)
{
    if (NEXT_IS("if"))
        return parse_if(lexer);

    if (NEXT_IS("while") || NEXT_IS("until"))
        return parse_while_clause(lexer);

    assert(0 && "not yet implented");
    return NULL;
}

//missing other shell command
static bool is_shell_command(struct queue *lexer)
{
    static const char *shell_command[] =
    {
        "if",
        "for",
        "while",
        "until",
        "case"
    };
    size_t size_array = sizeof(shell_command) / sizeof(char *);

    for (size_t i = 0; i < size_array; ++i)
    {
        if (NEXT_IS(shell_command[i]))
            return true;
    }
    return false;
}


static int parse_io_number(struct queue *lexer)
{
    if (!NEXT_IS_NUMBER())
        return -1;

    struct token_lexer *token = token_lexer_head(lexer);
    char *cpy = strdup(token->data);
    EAT();

    int fd = atoi(cpy);
    free(cpy);
    return fd;
}

static struct instruction *parse_redirection(struct queue *lexer)
{
    int fd = parse_io_number(lexer);

    if (fd == -1)
        fd = 1;

    enum token_parser_type type;

    if ((type = is_redirection(lexer)) == 0)
        return NULL;

    struct token_lexer *token = token_lexer_head(lexer);
    char *cpy = strdup(token->data);

    free(cpy);
    EAT(); //eat the redirection token

    token = token_lexer_head(lexer);

    if (token->type != TOKEN_OTHER)
        return build_instruction(type, build_redirection(fd, NULL));

    char *file = strdup(token->data);
    EAT(); //eat file token

    return build_instruction(type, build_redirection(fd, file));
}

//maybe write it better ?
static bool next_is_end_of_instruction(struct queue *lexer)
{
    struct token_lexer *token = token_lexer_head(lexer);
    if (token == NULL)
        return true;

    if (strcmp(token->data, "\n") == 0)
        return true;

    if (is_redirection(lexer))
        return true;

    if (NEXT_IS_NUMBER())
        return true;

    return token->type == TOKEN_END_OF_INSTRUCTION
            || token->type == TOKEN_EOF
            || token->type == TOKEN_OPERATOR;
}

static struct command_container *build_simple_command(char *simple_command,
                                                struct array_list *parameters
)
{
    return command_create(simple_command, parameters);
}

//simplified version of the grammar
//here doesn't call eat cause we need the token data
static struct instruction *parse_simple_command(struct queue *lexer)
{
    if (!NEXT_IS_OTHER() && !NEXT_IS_ASSIGNEMENT())
        return NULL;

    if (NEXT_IS_ASSIGNEMENT())
    {
        EAT();
        return build_instruction(TOKEN_COMMAND, NULL);
    }

    struct token_lexer *token = token_lexer_pop(lexer);
    char *simple_command_str = token->data;
    free(token);

    struct array_list *parameters = array_list_init();

    while (!next_is_end_of_instruction(lexer))
    {
        token = token_lexer_pop(lexer);
        array_list_append(parameters, token->data);
        free(token);
    }

    struct instruction *command = build_instruction(TOKEN_COMMAND,
                                                    build_simple_command(
                                                            simple_command_str,
                                                            parameters));

    free(parameters->content);
    free(parameters);//not destroy array_list cause we need it's content
    return command;
}

static bool redirection_not_valid(struct instruction *redirection)
{
    struct redirection *redir = redirection->data;
    return redir->file == NULL;
}

//only handle shell command and simple command
static struct instruction* parse_command(struct queue *lexer)
{
    if (is_redirection(lexer))
    {
        struct instruction *redirection = parse_redirection(lexer);
        struct redirection *redirect = redirection->data;
        redirect->to_redirect = NULL;
        return redirection;
    }

    struct instruction *command = NULL;

    if (is_shell_command(lexer))
        command = parse_shell_command(lexer);
    else
    {
        if (!NEXT_IS_OTHER())
            return NULL;
        command = parse_simple_command(lexer);
    }

    struct instruction *redirection = parse_redirection(lexer);

    if (redirection)
    {
        //to make difference between no redirection and bad grammar
        if (redirection_not_valid(redirection))
            return free_instructions(2, command, redirection);

        struct redirection *redirect = redirection->data;
        redirect->to_redirect = command;

        return redirection;
    }

    return command;
}

//not exactly grammar
static struct instruction *parse_pipeline(struct queue *lexer)
{
    struct instruction *command = parse_command(lexer);

    return command;
}

//not exactly grammar
//for now handle the * recursively
static struct instruction *parse_and_or(struct queue *lexer)
{
    struct instruction *left = NULL;
    if ((left = parse_pipeline(lexer)) == NULL)
        return NULL;

    struct instruction *right = NULL;
    struct instruction *root = left;

    while (NEXT_IS("||") || NEXT_IS("&&"))
    {
        struct token_lexer *operator = token_lexer_pop(lexer);
        enum token_parser_type type;
        if (strcmp(operator->data, "||") == 0)
            type = TOKEN_OR;
        else
            type = TOKEN_AND;

        token_lexer_free(&operator);
        while (NEXT_IS("\n"))
            EAT();

        if ((right = parse_pipeline(lexer)) == NULL)
            return free_instructions(1, left);

        root = build_instruction(type, build_and_or(root, right));
    }
    return root;
}


// grammar implemented recursively
// that's why there is no free there cause will return null before allocating
static struct instruction *parse_compound_list_break(struct queue *lexer)
{
    while (NEXT_IS("\n"))
        EAT();

    struct instruction *and_or = NULL;

    if ((and_or = parse_and_or(lexer)) == NULL)
        return NULL;

    if (NEXT_IS(";") || NEXT_IS("\n") || NEXT_IS("&"))
    {
        EAT();
        and_or->next = parse_compound_list_break(lexer);
    }

    while (NEXT_IS("\n"))
        EAT();
    return and_or;
}


static struct while_instruction *build_while_instruction(struct instruction *cond,
                                            struct instruction *to_do)
{
    struct while_instruction *while_i = 
                        xmalloc(sizeof(struct while_instruction));
    while_i->conditions = cond;
    while_i->to_execute = to_do;
    return while_i;
}


static struct instruction *parse_do_groupe(struct queue *lexer)
{
    if (!NEXT_IS("do"))
        return NULL;

    EAT();
    struct instruction *do_group = parse_compound_list_break(lexer);

    if (!NEXT_IS("done"))
        return free_instructions(1, do_group);

    EAT();
    return do_group;
}


static struct instruction *parse_while_clause(struct queue *lexer)
{
    if (!NEXT_IS("while") && !NEXT_IS("until"))
        return NULL;

    enum token_parser_type type = TOKEN_WHILE;

    if (NEXT_IS("until"))
        type = TOKEN_UNTIL;

    EAT();

    struct instruction *condition = parse_compound_list_break(lexer);
    struct instruction *to_execute = parse_do_groupe(lexer);

    if (!to_execute)
        return free_instructions(1, condition);

    return build_instruction(type,
                build_while_instruction(condition, to_execute));
}



static struct if_instruction *build_if_instruction(
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


static struct instruction *parse_else_clause(struct queue *lexer)
{
    if (NEXT_IS("else"))
    {
        EAT();
        return parse_compound_list_break(lexer);
    }
    else
    {
        EAT();
        struct instruction *conditions = NULL;
        struct instruction *to_execute = NULL;
        struct instruction *another_else = NULL;

        conditions = parse_compound_list_break(lexer);

        if (conditions == NULL || !NEXT_IS("then"))
            return free_instructions(1, conditions);
        EAT();

        if ((to_execute = parse_compound_list_break(lexer)) == NULL)
            return free_instructions(1, conditions);

        if (NEXT_IS("else") || NEXT_IS("elif"))
        {
            if ((another_else = parse_else_clause(lexer)) == NULL)
                return free_instructions(2, conditions, to_execute);
        }

        return build_instruction(TOKEN_IF, build_if_instruction(conditions,
                                                        to_execute,
                                                        another_else));
    }
}


static struct instruction *parse_if(struct queue *lexer)
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

    if (NEXT_IS("else") || NEXT_IS("elif"))
    {
        if ((else_container = parse_else_clause(lexer)) == NULL)
            return free_instructions(2, conditions, to_execute);
    }

    if (!NEXT_IS("fi"))
        return free_instructions(3, conditions, to_execute, else_container);
    EAT();

    return build_instruction(TOKEN_IF, build_if_instruction(conditions,
                                                        to_execute,
                                                        else_container));
}


//for now doesn't handle comand;command
static struct instruction *parse_list(struct queue *lexer)
{
    struct instruction *and_or = parse_and_or(lexer);

    if (and_or && (NEXT_IS(";") || NEXT_IS("&")))
    {
        EAT();
        and_or->next = parse_list(lexer);
    }

    return and_or;
}


static struct instruction *parser_error(struct instruction *ast, int *error)
{
    *error = 1;
    destroy_tree(ast);
    return NULL;
}


//for now doesn't handle if end with ; or with &
//TOO LONG
struct instruction* parse_input(struct queue *lexer, int *is_end, int *error)
{
    if (NEXT_IS("\n"))
    {
        EAT();
        return NULL;
    }

    if (NEXT_IS_EOF())
    {
        EAT();
        *is_end = 1;
        return NULL;
    }

    struct instruction *ast = NULL;

    if ((ast = parse_list(lexer)) == NULL)
        return parser_error(ast, error);

    struct instruction *tmp_ast = ast;

    while (NEXT_IS(";") || NEXT_IS("&"))
    {
        EAT();
        if ((tmp_ast->next = parse_and_or(lexer)) == NULL)
            return free_instructions(1, ast);//if error we need to free all ast
        tmp_ast = tmp_ast->next;
    }

    if (NEXT_IS("\n"))
    {
        EAT();
        return ast;
    }
    else if (NEXT_IS_EOF())
    {
        EAT();
        *is_end = 1;
        return ast;
    }
    else
        return parser_error(ast, error);
}