/** @file
* @brief build ast functions
* @author Coder : nicolas.blin & cloe.lacombe
* @author Tester : pierrick.made & zakaria.ben-hallal
* @author Reviewer : pierrick.made & zakaria.ben-hallal
*/

#pragma once

struct and_or_instruction* build_and_or(struct instruction *left,
                                    struct instruction *right);

struct case_clause *build_case_clause(char *pattern);

struct for_instruction *build_for_instruction(
                                            char *var_name,
                                            struct array_list *var_values,
                                            struct instruction *to_execute);

struct instruction* build_funcdef_instruction(void);

struct if_instruction *build_if_instruction(
                                        struct instruction *conditions,
                                        struct instruction *to_execute,
                                        struct instruction *else_container);

struct instruction *build_instruction(enum token_parser_type type,
                                                            void *input_instr);

struct pipe_instruction* build_pipe(struct instruction *left,
                                    struct instruction *right);

struct redirection *build_redirection(int fd, char *file);


struct command_container *build_simple_command(char *simple_command,
                                                struct array_list *parameters);

struct while_instruction *build_while_instruction(
                                            struct instruction *cond,
                                            struct instruction *to_do);