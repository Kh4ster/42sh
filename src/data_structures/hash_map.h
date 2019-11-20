/**
* @author Coder : nicolas.blin
* @author Tester : nicolas.blin
* @author Reviewer :
*/

#pragma once

#define NB_SLOTS 512

#include "../parser/parser.h"

/**
* @struct hash_slot
* @brief a slot in the hashmap
*/
struct hash_slot
{
    char *key;
    struct instruction *ast;
    struct hash_slot *next;
};

/**
* @struct hash_map
* @brief an hash_map
*/
struct hash_map
{
    struct hash_slot slots[NB_SLOTS];
    size_t size;
};

/**
* @brief init hash_map
* @param s the hash_map to init
* @param size number of slots, usefull for testing with few slots
* @relates hash_map
*/
void hash_init(struct hash_map *s, size_t size);

/**
* @brief insert ast combined with its name in hash_map
* @param set the hash_map
* @param key the name of the function
* @param ast the ast of the function
* @relates hash_map
*/
void hash_insert(struct hash_map *set,
                char *key,
                struct instruction *ast);

/**
* @brief return the function matched to its name
* @param set the hash_map
* @param key the name of the function
* @return the ast found : success; NULL : fail
* @relates hash_map
*/
struct instruction* hash_find(struct hash_map *set, char *key);

/**
* @brief free content of hashmap (hashmap declared on the stack)
* @param set the hash_map
* @relates hash_map
*/
void hash_free(struct hash_map *s);
