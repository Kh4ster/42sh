#include <stdio.h>
#include <err.h>

#include "ast.h"
#include "../parser.h"
#include "ast_print.h"
#include "../../execution_handling/command_container.h"

static void __print_ast(struct instruction *ast, FILE *file);

static void print_if_clause(struct if_instruction *if_clause, FILE *file)
{
    fprintf(file, "if -> ");
    __print_ast(if_clause->conditions, file);
    fprintf(file, " [label=\"conditions\"];\n");
    fprintf(file, "if -> ");
    __print_ast(if_clause->to_execute, file);
    fprintf(file, " [label=\"then\"];\n");

    if (if_clause->else_container)
    {
        fprintf(file, "if -> ");
        __print_ast(if_clause->else_container, file);
        fprintf(file, " [label=\"else\"];\n");
    }
}


static void print_command(struct command_container *cmd, FILE *file)
{
    fprintf(file, "%s", cmd->command);

    for (size_t i = 1; cmd->params[i]; i++)
        fprintf(file, "_%s", cmd->params[i]);
}


static void print_and_or(struct instruction *a_o, FILE *file)
{
    struct and_or_instruction *a_o_i = a_o->data;

    if (a_o->type == TOKEN_AND)
    {
        fprintf(file, "&& -> ");
        __print_ast(a_o_i->left, file);
        fprintf(file, "\n");
        fprintf(file, "&& -> ");
        __print_ast(a_o_i->right, file);
        fprintf(file, "\n");
    }
    else
    {
        fprintf(file, "|| -> ");
        __print_ast(a_o_i->left, file);
        fprintf(file, "\n");
        fprintf(file, "|| -> ");
        __print_ast(a_o_i->right, file);
        fprintf(file, "\n");
    }
}


#if 0
static void print_redirection(struct redirection *redirect, FILE *file)
{
}
#endif

static void __print_ast(struct instruction *ast, FILE *file)
{
    if (!ast)
        return;

    switch (ast->type)
    {
        case TOKEN_AND:
        case TOKEN_OR:
            print_and_or(ast, file);
            break;
        case TOKEN_IF:
            print_if_clause(ast->data, file);
            break;
        case TOKEN_COMMAND:
            print_command(ast->data, file);
            break;
        default:
            return;
    }
}


extern void print_ast(struct instruction *ast)
{
    if (!ast)
        return;

    FILE *dot_ast = fopen("ast.dot", "a+");

    if (!dot_ast)
        warn("Could not open file ast.dot");

    fprintf(dot_ast, "digraph ast {\n");
    __print_ast(ast, dot_ast);
    fprintf(dot_ast, "}\n");
}
