#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <err.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"
#include "ast_build.h"
#include "ast/ast.h"
#include "ast/destroy_tree.h"
#include "../lexer/token_lexer.h"
#include "../memory/memory.h"
#include "../execution_handling/command_container.h"
#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"
#include "../redirections_handling/redirect.h"
#include "../data_structures/data_string.h"

struct and_or_instruction* build_and_or(struct instruction *left,
                                    struct instruction *right
)
{
    struct and_or_instruction *and_or = xmalloc(sizeof(*and_or));
    and_or->left = left;
    and_or->right = right;
    return and_or;
}

struct case_clause *build_case_clause(char *pattern)
{
    struct case_clause *case_clause = xmalloc(sizeof(struct case_clause));
    case_clause->pattern = strdup(pattern);
    case_clause->items = array_list_init();
    return case_clause;
}

struct for_instruction *build_for_instruction(
                                            char *var_name,
                                            struct array_list *var_values,
                                            struct instruction *to_execute
)
{
    struct for_instruction *new_for = xmalloc(sizeof(struct for_instruction));
    new_for->var_name = var_name;
    new_for->var_values = var_values;
    new_for->to_execute = to_execute;
    return new_for;
}

struct instruction* build_funcdef_instruction(void)
{
    struct instruction *instruction = xmalloc(sizeof(*instruction));
    instruction->data = NULL;
    instruction->type = TOKEN_FUNC_DEF;
    instruction->next = NULL;
    return instruction;
}

struct if_instruction *build_if_instruction(
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

struct pipe_instruction* build_pipe(struct instruction *left,
                                    struct instruction *right
)
{
    struct pipe_instruction *pipe = xmalloc(sizeof(*pipe));
    pipe->left = left;
    pipe->right = right;
    return pipe;
}

struct redirection *build_redirection(int fd, char *file)
{
    struct redirection *redirect = xmalloc(sizeof(struct redirection));
    redirect->to_redirect = NULL;
    redirect->fd = fd;
    redirect->file = file;
    redirect->temp_file = NULL;
    return redirect;
}

struct command_container *build_simple_command(char *simple_command,
                                                struct array_list *parameters
)
{
    return command_create(simple_command, parameters);
}

struct while_instruction *build_while_instruction(
                                            struct instruction *cond,
                                            struct instruction *to_do
)
{
    struct while_instruction *while_i =
                        xmalloc(sizeof(struct while_instruction));
    while_i->conditions = cond;
    while_i->to_execute = to_do;
    return while_i;
}