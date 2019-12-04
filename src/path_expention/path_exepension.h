/** @file
* @brief function to handle *, ? and [] variables
* @author Coder : cloe.lacombe
* @author Tester :
* @author Reviewer :
* @author Integrator :
*/

#pragma once

#include "../data_structures/array_list.h"

struct path_globbing {
    int nb_matchs;
    struct array_list *matches;
};

extern void destroy_path_glob(struct path_globbing *p_glob);

extern struct path_globbing *sh_glob(char *pattern);

int is_path_expansion(char *pattern);