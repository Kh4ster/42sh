/**
* @author Coder : nicolas.blin
* @author Tester : nicolas.blin
* @author Reviewer : pierrick.made
*/

#pragma once

#define NB_SLOTS 512

#include "../parser/parser.h"

typedef int (*builtin)(char*[]);

/**
* @enum data_type
* @brief to know which type is in the hashmap, change the way we free it
*/
enum data_type
{
    STRING = 0, /**< @brief string type */
    AST /**< @brief ast type for builtin and functions */
};

/**
* @struct hash_slot
* @brief a slot in the hashmap, can contain ast, string and builtins
*/
struct hash_slot
{
    enum data_type data_type; /**< @brief string or ast */
    void *data; /**< @brief string or ast contained */
    builtin builtin; /**< @brief the code of the builtin */
    struct hash_slot *next; /**< @brief to handle same hash */
    char *key; /**< @brief key as a string */
};

/**
* @struct hash_map
* @brief an hash_map
*/
struct hash_map
{
    struct hash_slot slots[NB_SLOTS]; /**< @brief array of slots */
    size_t size; /**< @brief current size of hashmap */
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
* @param data da data
* @param data_type if it's string
* @relates hash_map
*/
void hash_insert(struct hash_map *set,
                char *key,
                void *data,
                enum data_type data_type);
/**
* @brief insert a function pointer combined with its name in hash_map
* @param set the hash_map
* @param key the name of the function
* @param builtin the function pointer
* @relates hash_map
*/
void hash_insert_builtin(struct hash_map *set,
                char *key,
                builtin builtin);

/**
* @brief return the function (as an instruction) matched to its name
* @param set the hash_map
* @param key the name of the function
* @return the instruction : success; NULL : fail
* @relates hash_map
*/
void* hash_find(struct hash_map *set, char *key);

/**
* @brief return the builtin (as a function pointer) matched to its name
* @param set the hash_map
* @param key the name of the function
* @return the function pointer : success; NULL : fail
* @relates hash_map
*/
builtin hash_find_builtin(struct hash_map *set, char *key);

/**
* @brief free content of hashmap (hashmap declared on the stack)
* @param s the hash_map
* @details ONLY TO BE CALLED ON THE FUNCTION HASH_MAP
* @details IN THE BUILTIN HASH_MAP NOTHING IS TO BE FREED
* @relates hash_map
*/
void hash_free(struct hash_map *s);

/**
* @brief remove an element from the hashèmap
* @param set the hash_map
* @param key the key
* @relates hash_map
*/
int hash_remove(struct hash_map *set, char *key);