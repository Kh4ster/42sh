/** @file
* @brief Wrapper of memory calls to handle no memory case or other
* @author Coder : nicolas.blin
*/

#pragma once

# include <stddef.h>
# include <stdlib.h>

/**
* @brief malloc wrapper
* @param size size to allocate
* @return void* : success, errx : fail
*/
void *xmalloc(size_t size);

/**
* @brief calloc wrapper
* @param size size to allocate per block
* @param nmemb number of blocks
* @return void* : success, errx : fail
*/
void *xcalloc(size_t nmemb, size_t size);

/**
* @brief realloc wrapper
* @param ptr old pointer that needs to be reallocated
* @param size new size to allocate
* @return void* : success, errx : fail
*/
void *xrealloc(void *ptr, size_t size);