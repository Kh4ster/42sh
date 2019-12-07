#ifndef READ_H
# define READ_H

#include "../data_structures/data_string.h"

/**
* @brief read everything and handle signal
* @param string string to put result in
* @param fd_in to fd to read one
*/
int xread(struct string *string, int fd_in);

#endif /* ! READ_H */