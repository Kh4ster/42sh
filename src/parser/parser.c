#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <err.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"
#include "parser_utils.h"
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

#define NEXT_IS(X) next_is(lexer, X)
#define EAT() eat_token(lexer)
#define NEXT_IS_EOF() next_is_eof(lexer)
#define NEXT_IS_OTHER() next_is_other(lexer)
#define NEXT_IS_ASSIGNEMENT() next_is_assignement(lexer)
#define NEXT_IS_NUMBER() next_is_number(lexer)

#define IFS " \t\n"

// free all instructions passed and returns NULL
static void *free_instructions(size_t nb_param, ...)
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

static void eat_token(struct queue *lexer)
{
    struct token_lexer *to_free = token_lexer_pop(lexer);
    token_lexer_free(&to_free);
}

static struct instruction *parse_if(struct queue *lexer);
static struct instruction *parse_while_clause(struct queue *lexer);
static struct instruction *parse_compound_list_break(struct queue *lexer);
static struct instruction *parse_for(struct queue *lexer);
static struct instruction *parse_case_rule(struct queue *lexer);

static struct instruction *parse_control_structure(struct queue *lexer)
{
    if (NEXT_IS("if"))
        return parse_if(lexer);

    if (NEXT_IS("while") || NEXT_IS("until"))
        return parse_while_clause(lexer);

    if (NEXT_IS("for"))
        return parse_for(lexer);

    if (NEXT_IS("case"))
        return parse_case_rule(lexer);

    return NULL; //impossible
}

static struct instruction *parse_shell_command(struct queue *lexer)
{
    if (next_is_control_structure(lexer))
        return parse_control_structure(lexer);

    struct instruction *to_execute = NULL;
    if (NEXT_IS("{"))
    {
        EAT();
        if ((to_execute = parse_compound_list_break(lexer)) == NULL)
            return NULL;

        if (!NEXT_IS("}"))
            return free_instructions(1, to_execute);
        EAT();

        return to_execute;
    }
    else if (NEXT_IS("("))
    {
        EAT();
        if ((to_execute = parse_compound_list_break(lexer)) == NULL)
            return NULL;

        if (!NEXT_IS(")"))
            return free_instructions(1, to_execute);
        EAT();

        return to_execute;
    }
    return NULL;
}

static struct instruction *parse_funcdec(struct queue *lexer)
{
    struct instruction *to_execute = NULL;

    if (NEXT_IS("function"))
        EAT();

    if (!NEXT_IS_OTHER())
        return NULL;

    struct token_lexer *func_name = token_lexer_pop(lexer);

    if (!NEXT_IS("("))
    {
        token_lexer_free(&func_name);
        return NULL;
    }
    EAT();

    if (!NEXT_IS(")"))
    {
        token_lexer_free(&func_name);
        return NULL;
    }
    EAT();

    while (NEXT_IS("\n"))
        EAT();

    if ((to_execute = parse_shell_command(lexer)) == NULL)
    {
        token_lexer_free(&func_name);
        return NULL;
    }

    hash_insert(g_env.functions, func_name->data, to_execute, AST);
    free(func_name->data);
    free(func_name);
    return  build_funcdef_instruction();
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

static struct instruction *__parse_redirection(struct queue *lexer)
{
    if (lexer->size == 0)
        return NULL;

    int fd = parse_io_number(lexer);

    enum token_parser_type type;

    if (lexer->size == 0)
        return NULL;

    if ((type = is_redirection(lexer)) == 0)
        return NULL;

    if (fd == -1)
    {
        if (type == TOKEN_REDIRECT_RIGHT || type == TOKEN_HEREDOC
            || type == TOKEN_REDIRECT_READ_WRITE || type == TOKEN_HEREDOC_MINUS)
            fd = 0;
        else
            fd = 1;
    }
    EAT(); //eat the redirection token

    if (next_is_end_of_instruction(lexer))
        return build_instruction(type, build_redirection(fd, NULL));

    struct token_lexer *token = token_lexer_head(lexer);
    char *file = strdup(token->data);
    EAT(); //eat file token

    return build_instruction(type, build_redirection(fd, file));
}


static bool redirection_not_valid(struct instruction *redirection);

static struct instruction *parse_redirection(struct queue *lexer,
                                        struct instruction *command
)
{
    struct instruction *redirection = __parse_redirection(lexer);

    if (!redirection)
        return command;

    struct instruction *tmp = redirection;
    struct instruction *redirection2 = NULL;

    while (redirection)
    {
        //to make difference between no redirection and bad grammar
        if (redirection_not_valid(redirection))
            return free_instructions(2, command, redirection);

        if (redirection->type == TOKEN_HEREDOC
                || redirection->type == TOKEN_HEREDOC_MINUS)
            redirections_handling(redirection, 0);

        struct redirection *redirect = redirection->data;
        redirect->to_redirect = command;

        if (redirection2)
        {
            struct redirection *redirect2 = redirection2->data;
            redirect2->to_redirect = redirection;
        }

        redirection2 = redirection;

        if (lexer->size > 0)
            redirection = __parse_redirection(lexer);
        else
            redirection = NULL;
    }

    return tmp;
}

static struct instruction *add_command_redirection(
                    struct instruction *redirection, struct instruction *cmd
)
{
    struct redirection *redirect = redirection->data;

    if (!redirect->to_redirect)
        redirect->to_redirect = cmd;
    else
        redirect->to_redirect = add_command_redirection(redirect->to_redirect,
                                                    cmd);

    return redirection;
}

//for variable we just want 1 space between variable
static char *slice_expansion(char *var)
{
    if (var[0] == '\0') //empty after expansion
        return var;

    char *beg = var;
    struct string *sliced_value = string_init();
    char *slice;
    string_append(sliced_value, ((slice = strtok_r(var,
            hash_find(g_env.variables, "IFS"), &var)) == NULL)
                ? " " : slice);

    while ((slice = strtok_r(var,
                        hash_find(g_env.variables, "IFS"), &var)) != NULL)
    {
        slice--;
        *slice = ' '; //insert one space juste before
        string_append(sliced_value, slice);
    }
    char *content = string_get_content(&sliced_value);
    free(beg);
    return content;
}

static void add_variable(char *var)
{
    char *name = strtok_r(var, "=", &var);
    char *value = strtok_r(NULL, "=", &var);

    if (value == NULL) //empty var value
        value = strdup("");
    else
        value = slice_expansion(scan_for_expand(value, false, NULL));

    hash_insert(g_env.variables, name, value, STRING);
    free(value);
}

static struct instruction *parse_simple_command(struct queue *lexer)
{
    struct instruction *redirection = parse_redirection(lexer, NULL);

    if (!NEXT_IS_OTHER() && !NEXT_IS_ASSIGNEMENT() && redirection == NULL)
    {
        return NULL;
    }

    if (next_is_end_of_instruction(lexer))
        return redirection;

    if (NEXT_IS_ASSIGNEMENT())
    {
        struct token_lexer *token = token_lexer_head(lexer);
        if (token->data[0] != '=') //just =value, is considered a command
        {
            add_variable(token->data);
            EAT();
            return build_instruction(TOKEN_VAR_DECLARATION, NULL);
        }
    }

    struct token_lexer *token = token_lexer_pop(lexer);
    char *simple_command_str = token->data;
    free(token);

    struct array_list *parameters = array_list_init();

    while (!next_is_end_of_instruction(lexer))
    {
        if (is_redirection(lexer) || NEXT_IS_NUMBER())
        {
            struct instruction *redirection2 =
                                parse_redirection(lexer, NULL);

            if (redirection)
                redirection = add_command_redirection(redirection, redirection2);
            else
                redirection = redirection2;

            if (!redirection2)
            {
                array_list_destroy(parameters);
                free(simple_command_str);
                return NULL;
            }

            continue;
        }

        token = token_lexer_pop(lexer);
        array_list_append(parameters, token->data);
        free(token);
    }

    struct instruction *command = build_instruction(TOKEN_COMMAND,
                                                    build_simple_command(
                                                            simple_command_str,
                                                            parameters));

    if (redirection)
    {
        if (redirection_not_valid(redirection))
            return NULL;
        command = add_command_redirection(redirection, command);
    }

    free(parameters->content);
    free(parameters);//not destroy array_list cause we need it's content
    free(simple_command_str); //now it's duped in the command struct
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
    struct instruction *command = NULL;

    if (is_shell_command(lexer))
        command = parse_shell_command(lexer);
    else if (is_function(lexer))
        command = parse_funcdec(lexer);
    else //simple command
    {
        if (!NEXT_IS_OTHER() && !is_redirection(lexer)
                                                    && !NEXT_IS_ASSIGNEMENT())
            return NULL;
        command = parse_simple_command(lexer);
    }

    return parse_redirection(lexer, command);
}

static struct instruction *parse_pipeline(struct queue *lexer)
{
    struct instruction *left = NULL;
    struct instruction *right = NULL;
    bool not = false;

    if (NEXT_IS("!"))
    {
        EAT();
        not = true;
    }

    if ((left = parse_command(lexer)) == NULL)
        return NULL;

    if (not)
        left->type = TOKEN_NOT;

    struct instruction *root = left;
    while (NEXT_IS("|"))
    {
        EAT();

        while (NEXT_IS("\n"))
            EAT();

        if ((right = parse_command(lexer)) == NULL)
            return free_instructions(1, root);

        root = build_instruction(TOKEN_PIPE, build_pipe(root, right));
    }

    return root;
}

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
            return free_instructions(1, root);

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

    if (NEXT_IS(";") || NEXT_IS("&") || NEXT_IS("\n"))
    {
        EAT();
        and_or->next = parse_compound_list_break(lexer);
    }

    while (NEXT_IS("\n"))
        EAT();

    return and_or;
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

static struct array_list *parse_for_var_values(struct queue *lexer)
{
    struct array_list *var_values = NULL;

    while (NEXT_IS_OTHER() && !NEXT_IS("\n"))
    {
        if (var_values == NULL)
            var_values = array_list_init();
        array_list_append(var_values, strdup(token_lexer_head(lexer)->data));
        EAT();
    }

    return var_values;
}

static struct instruction *parse_for(struct queue *lexer)
{
    if (!NEXT_IS("for"))
        return NULL;
    EAT();

    if (next_is_end_of_instruction(lexer))
        return NULL;

    char *var_name = strdup(token_lexer_head(lexer)->data);
    struct array_list *var_values = NULL;
    EAT();

    if (NEXT_IS(";"))
        EAT();
    else
    {
        while (NEXT_IS("\n"))
            EAT();
        if (NEXT_IS("in"))
        {
            EAT();
            var_values = parse_for_var_values(lexer);
            if (!NEXT_IS(";") && !NEXT_IS("\n"))
            {
                array_list_destroy(var_values);
                free(var_name);
                return NULL;
            }
            EAT();
        }
    }
    while (NEXT_IS("\n"))
        EAT();

    return build_instruction(TOKEN_FOR, build_for_instruction(var_name,
            var_values, parse_do_groupe(lexer)));
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

static struct case_item *init_case_item(void)
{
    struct case_item *item = xmalloc(sizeof(struct case_item));
    item->patterns = array_list_init();
    item->to_execute = NULL;
    return item;
}

static void *destroy_case_item(struct case_item *item)
{
    array_list_destroy(item->patterns);
    free(item);
    return NULL;
}

static struct case_item *parse_case_item(struct queue *lexer, int *error)
{
    if (NEXT_IS("("))
        EAT();

    if (NEXT_IS("esac"))
        return NULL;

    struct token_lexer *token = token_lexer_pop(lexer);

    if (token->type != TOKEN_OTHER)
    {
        token_lexer_free(&token);
        *error = 1;
        return NULL;
    }

    struct case_item *item = init_case_item();
    array_list_append(item->patterns, strdup(token->data));
    token_lexer_free(&token);

    while (NEXT_IS("|"))
    {
        EAT();
        token = token_lexer_pop(lexer);

        if (!token)
        {
            *error = 1;
            return destroy_case_item(item);
        }

        array_list_append(item->patterns, strdup(token->data));
        token_lexer_free(&token);
    }

    if (!NEXT_IS(")"))
    {
        *error = 1;
        return destroy_case_item(item);
    }

    EAT();
    while (NEXT_IS("\n"))
        EAT();

    item->to_execute = parse_compound_list_break(lexer);
    struct instruction *tmp = item->to_execute;

    while (tmp && !next_is_end_of_case_item(lexer))
    {
        tmp->next = parse_compound_list_break(lexer);
        tmp = tmp->next;
    }

    return item;
}


static struct instruction *parse_case_clause(struct queue *lexer,
                                struct case_clause *case_clause, int *error
)
{
    struct case_item *first_item = parse_case_item(lexer, error);

    if (*error)
        return build_instruction(TOKEN_CASE, case_clause);

    if (first_item)
        array_list_append(case_clause->items, first_item);

    while (!NEXT_IS("esac"))
    {
        while (NEXT_IS("\n"))
            EAT();

        if (!NEXT_IS(";;"))
        {
            *error = 1;
            return build_instruction(TOKEN_CASE, case_clause);
        }

        EAT();

        while (NEXT_IS("\n"))
            EAT();

        first_item = parse_case_item(lexer, error);

        if (*error)
            return build_instruction(TOKEN_CASE, case_clause);

        if (first_item)
            array_list_append(case_clause->items, first_item);
    }

    if (NEXT_IS(";;"))
        EAT();

    while (NEXT_IS("\n"))
        EAT();

    return build_instruction(TOKEN_CASE, case_clause);
}

static void skip_new_line(struct queue *lexer)
{
    while (NEXT_IS("\n"))
        EAT();
}

static struct instruction *parse_case_rule(struct queue *lexer)
{
    if (!NEXT_IS("case"))
        return NULL;

    EAT();
    struct token_lexer *token = token_lexer_pop(lexer);

    if (!token || token->type != TOKEN_OTHER)
    {
        if (token)
            token_lexer_free(&token);
        return NULL;
    }

    skip_new_line(lexer);

    if (!NEXT_IS("in"))
    {
        token_lexer_free(&token);
        return NULL;
    }

    EAT();
    struct case_clause *clause = build_case_clause(token->data);
    token_lexer_free(&token);

    while (NEXT_IS("\n"))
        EAT();

    int error = 0;
    struct instruction *case_rule = parse_case_clause(lexer, clause, &error);

    if (error)
        return free_instructions(1, case_rule);

    if (!NEXT_IS("esac"))
        return free_instructions(2, clause, case_rule);

    EAT();
    return case_rule;
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

static struct instruction *parse_list(struct queue *lexer)
{
    struct instruction *and_or = parse_and_or(lexer);
    struct instruction *tmp = and_or;

    while (NEXT_IS(";") || NEXT_IS("&"))
    {
        EAT();

        if (!tmp)
            return free_instructions(1, and_or);

        tmp->next = parse_and_or(lexer);
        tmp = tmp->next;
    }

    return and_or;
}

static struct instruction *parser_error(struct instruction *ast, int *error)
{
    *error = 1;
    destroy_tree(ast);
    return NULL;
}

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
