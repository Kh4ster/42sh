#pragma once
#include "../data_structures/array_list.h"

struct command_container
{
    char *command;
    char **params;
};

struct command_container* command_init(size_t nb_args, char *command, ...);
void command_destroy(struct command_container **command_container);
struct command_container *command_create(char *cmd, struct array_list *list);
