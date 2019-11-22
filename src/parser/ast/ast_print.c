#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "../parser.h"
#include "ast_print.h"
#include "../../execution_handling/command_container.h"
#include "../../memory/memory.h"

struct env_ast
{
    size_t nb_if;
    size_t nb_cmd;
    size_t nb_while;
    size_t nb_until;
    size_t nb_redirections;
    size_t nb_and_or;
    char *labels;
};

struct env_ast g_env_ast;

static void env_ast_init(void)
{
    g_env_ast.nb_if = 1;
    g_env_ast.nb_cmd = 1;
    g_env_ast.nb_while = 1;
    g_env_ast.nb_until = 1;
    g_env_ast.nb_redirections = 1;
    g_env_ast.nb_and_or = 1;
    g_env_ast.labels = "";
}


static void __print_ast(struct instruction *ast, FILE *file, int flag);


static void handle_special_case(struct instruction *next, FILE *file,
                            char *format_from, char *label)
{
    if (next->type == TOKEN_IF)
        fprintf(file, "%s -> if_%ld [label=%s]\n", format_from, g_env_ast.nb_if,
                                        label);
    #if 0
    if (next->type == TOKEN_WHILE)
        fprintf(file, "%s -> while_%ld [label=%s]\n",format_from ,
                        g_env_ast.nb_while, label);

    if (next->type == TOKEN_UNTIL)
        fprintf(file, "%s -> until_%ld [label=%s]\n", format_from,
                                          g_env_ast.nb_until, label);
    #endif
    else
        fprintf(file, "%s -> redirection_%ld [label=%s]\n", format_from,
                                g_env_ast.nb_redirections, label);
}

static void print_if_clause(struct if_instruction *if_clause, FILE *file)
{
    char *format_if = NULL;
    int ouais = asprintf(&format_if, "if_%ld", g_env_ast.nb_if);
    g_env_ast.nb_if++;

    fprintf(file, "%s -> ", format_if);
    __print_ast(if_clause->conditions, file, 0);
    fprintf(file, " [label=\"conditions\"];\n");

    if (if_clause->to_execute->type != TOKEN_COMMAND)
    {
        handle_special_case(if_clause->to_execute, file, format_if, "then");
        __print_ast(if_clause->to_execute, file, 0);
    }
    else
    {
        fprintf(file, "%s -> ", format_if);
        __print_ast(if_clause->to_execute, file, 0);
        fprintf(file, " [label=\"then\"];\n");
    }

    if (if_clause->else_container)
    {
        if (if_clause->else_container->type != TOKEN_COMMAND)
        {
            handle_special_case(if_clause->else_container, file, format_if,
                                                        "then");
            __print_ast(if_clause->else_container, file, 0);
        }
        else
        {
            fprintf(file, "%s -> ", format_if);
            __print_ast(if_clause->else_container, file, 0);
            fprintf(file, " [label=\"then\"];\n");
        }
    }

    char *new_env = NULL;
    ouais += asprintf(&new_env, "%s%s [label=\"if\"];\n", g_env_ast.labels,
                                                            format_if);
    free(g_env_ast.labels);
    g_env_ast.labels = new_env;
    free(format_if);
}


static void print_command(struct command_container *cmd, FILE *file, int flag)
{
    char *format = NULL;
    int ouais = asprintf(&format, "%s_%ld",cmd->command, g_env_ast.nb_cmd);
    g_env_ast.nb_cmd++;

    char *label_cmd = NULL;
    ouais += asprintf(&label_cmd, "%s", cmd->command);

    for (size_t i = 1; cmd->params[i]; i++)
    {
        char *new_label = NULL;
        ouais += asprintf(&new_label,"%s %s", label_cmd, cmd->params[i]);
        free(label_cmd);
        label_cmd = new_label;
    }

    fprintf(file, "%s", format);

    if (flag)
        fprintf(file, "\n");

    char *new_env = NULL;
    ouais += asprintf(&new_env, "%s%s[label=\"%s\"];\n",
            g_env_ast.labels,format, label_cmd);

    if (*g_env_ast.labels)
        free(g_env_ast.labels);

    g_env_ast.labels = new_env;

    free(format);
    free(label_cmd);
}


static void print_and_or(struct instruction *a_o, FILE *file)
{
    struct and_or_instruction *a_o_i = a_o->data;

    if (a_o->type == TOKEN_AND)
    {
        fprintf(file, "&& -> ");
        __print_ast(a_o_i->left, file, 1);
        fprintf(file, "\n");
        fprintf(file, "&& -> ");
        __print_ast(a_o_i->right, file, 1);
        fprintf(file, "\n");
    }
    else
    {
        fprintf(file, "|| -> ");
        __print_ast(a_o_i->left, file, 1);
        fprintf(file, "\n");
        fprintf(file, "|| -> ");
        __print_ast(a_o_i->right, file, 1);
        fprintf(file, "\n");
    }
}


static void print_redirection(struct redirection *redirect, FILE *file)
{
    char *format_redirection = NULL;
    int ouai = asprintf(&format_redirection, "redirection_%ld",
                                            g_env_ast.nb_redirections);
    g_env_ast.nb_redirections++;

    if (redirect->to_redirect->type != TOKEN_COMMAND)
    {
        handle_special_case(redirect->to_redirect, file, format_redirection,
                                            "redirect");
        __print_ast(redirect->to_redirect, file, 0);
    }
    else
    {
        fprintf(file, "%s -> ", format_redirection);
        __print_ast(redirect->to_redirect, file, 0);
        fprintf(file, " [label=redirect]\n");
    }

    fprintf(file, "%s -> fd_%ld [label=\"from FD\"]\n", format_redirection,
                                g_env_ast.nb_redirections);
    fprintf(file, "fd_%ld [label=\"%d\"]\n", g_env_ast.nb_redirections,
                                    redirect->fd);
    g_env_ast.nb_redirections++;

    fprintf(file, "%s -> %s_%ld [label=\"to file\"]\n", format_redirection,
                                    redirect->file, g_env_ast.nb_redirections);
    fprintf(file, "%s_%ld [label = %s]\n", redirect->file,
                    g_env_ast.nb_redirections, redirect->file);

    char *new_env = NULL;
    ouai += asprintf(&new_env, "%s%s[label=\"redirection\"];\n",
            g_env_ast.labels,format_redirection);

    if (*g_env_ast.labels)
        free(g_env_ast.labels);

    g_env_ast.labels = new_env;
    free(format_redirection);

}


static void __print_ast(struct instruction *ast, FILE *file, int flag)
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
            print_command(ast->data, file, flag);
            break;
        default:
            print_redirection(ast->data, file);
    }
    __print_ast(ast->next, file, 1);
}


extern void print_ast(struct instruction *ast)
{
    if (!ast)
        return;

    FILE *dot_ast = fopen("ast.dot", "w");

    if (!dot_ast)
    {
        warn("Could not open file ast.dot");
        return;
    }

    env_ast_init();
    fprintf(dot_ast, "digraph ast {\n");
    __print_ast(ast, dot_ast, 1);
    fprintf(dot_ast, "%s", g_env_ast.labels);
    fprintf(dot_ast, "}\n");
    free(g_env_ast.labels);
    fclose(dot_ast);
}
