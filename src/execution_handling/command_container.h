#pragma once

struct command_container
{
    char *command;
    char **params;
};

struct command_container* command_init(size_t nb_args, char *command, ...);
void command_destroy(struct command_container **command_container);
