/** @file
* @brief Handling command_container structure
* @author Coder : nicolas.blin & zakaria.ben-allal
* @author Tester : zakaria.ben-allal
* @author Reviewer : nicolas.blin
* @author Integrator :
*/

#pragma once

#include "../data_structures/array_list.h"

/**
* @struct command_container
* @brief structure containing command name and its parameters
*/

struct command_container
{
    char *command; /**< @brief command name */
    char **params; /**< @brief command parameters */
};

/**
* @brief returns an initialized command container
* @brief filled with given parameters(for unitary tests only)
* @param number of arguments
* @param command name string
* @return struct command_container
*/

struct command_container *command_init(size_t nb_args, char *command, ...);

/**
* @brief destroy allocated command_container structure
* @param command_container structure to destroy
*/

void command_destroy(struct command_container **command_container);

/**
* @brief create a command_container structure
* @brief from the command name and an array_list structure
* @param command name string
* @param array_list structure that will hold the command name & the parameters
* @return command_container structure
*/

struct command_container *command_create(char *cmd, struct array_list *list);
