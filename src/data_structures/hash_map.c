#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "hash_map.h"
#include "../parser/ast/destroy_tree.h"
#include "../memory/memory.h"

static size_t hash(const char *key, size_t table_size)
{
    static const int p = 31;

    size_t hash_value = 0;
    size_t p_pow = 1;
    for (size_t i = 0; key[i]; ++i) 
    {
        hash_value = (hash_value + (key[i] - 'a' + 1) * p_pow) % table_size;
        p_pow = (p_pow * p) % table_size;
    }
    return hash_value;
}

static struct hash_slot* get_slot(struct hash_map *set, char *key)
{
    size_t v = hash(key, set->size);
    struct hash_slot *s = &(set->slots[v]);
    return s;
}

void hash_insert(struct hash_map *set,
                char *key,
                struct instruction *ast
)
{
    assert(key != NULL);

    struct hash_slot *s = get_slot(set, key);

    if (s->key == NULL) //first time at this index
    {
        s->key = key;
        s->ast = ast;
    }
    else if (hash_find(set, key) == NULL) //if element not already in hashmap
    {
        struct hash_slot *new = xmalloc(sizeof(struct hash_slot));
        new->key = key;
        new->ast = ast;
        new->next = NULL;
        while (s->next != NULL)
            s = s->next;
        s->next = new;
    }
}

static int free_slot(struct hash_slot *s)
{
    free(s->key);
    destroy_tree(s->ast);
    s->key = NULL;
    s->ast = NULL;
    s->next = NULL;
    return 1;
}

static int handle_start(struct hash_slot *s)
{
    s->key = s->next->key;
    s->ast = s->next->ast;
    struct hash_slot *next = s->next;
    s->next = s->next->next;
    return free_slot(next);
}

int hash_remove(struct hash_map *set, char *key)
{
    assert(key != NULL);

    struct hash_slot *s = get_slot(set, key);

    if (s->key == NULL) //not in hashmap
        return 0;

    if (s->next == NULL && strcmp(key, s->key) == 0)//no chaining
        return free_slot(s);

    struct hash_slot *f = s;
    while (strcmp(key, s->key) != 0) // go through chaining
    {
        f = s;
        s = s->next;
        if (s == NULL)
            return 0;
    }

    if (s == f) //first element of chain
        return handle_start(s);

    f->next = s->next;
    return free_slot(s);
}

struct instruction* hash_find(struct hash_map *set, char *key)
{
    assert(key != NULL);

    struct hash_slot *s = get_slot(set, key);

    if (s->key == NULL) //key no present
        return NULL;

    while (strcmp(s->key, key) != 0)
    {
        s = s->next;
        if (s == NULL)
            return NULL;
    }

    return s->ast;
}

void hash_init(struct hash_map *s, size_t size)
{
    s->size = size;

    for (size_t i = 0; i < s->size; ++i)
    {
        s->slots[i].key = NULL;
        s->slots[i].next = NULL;
        s->slots[i].ast = NULL;
    }
}

void hash_free(struct hash_map *s)
{
    for (size_t i = 0; i < s->size; ++i)
    {
        struct hash_slot *current_slot = &(s->slots[i]);
        int first = 1;
        while (current_slot != NULL)
        {
            struct hash_slot *next_slot = current_slot->next;
            free_slot(current_slot);
            if (!first) //cause first slot not malloced
                free(current_slot);
            current_slot = next_slot;
            first = 0;
        }
    }
}