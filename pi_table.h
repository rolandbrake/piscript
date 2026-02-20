#ifndef PI_TABLE_H
#define PI_TABLE_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "list.h"

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct
{
    char *key;
    void *value;   // Generic value pointer
    uint64_t hash; // Precomputed hash
} ht_item;

typedef struct
{
    int size;       // Number of items in the table
    int capacity;   // Total capacity of the table
    size_t i_size;  // Size of each value type
    ht_item *items; // Array of items

    char **_keys;

    int _last;
} table_t;

// Create a table for values of size `i_size`
table_t *ht_create(size_t i_size);
void *ht_get(table_t *table, const char *key);
bool ht_set(table_t *table, const char *key, const void *value);
bool ht_put(table_t *table, const char *key, const void *value);
// bool ht_put(table_t *table, const char *key, const void *value);
bool ht_expand(table_t *table);
int ht_length(table_t *table);
int ht_last(table_t *table);
char **ht_keys(table_t *table);
void ht_free(table_t *table);

typedef struct
{
    char *key;       // Current key
    void *value;     // Current value
    table_t *_table; // Reference to the table
    size_t _index;   // Current index
} ht_iter;

ht_iter ht_iterator(table_t *table);
bool ht_next(ht_iter *it);
bool ht_hasNext(ht_iter *it);
void ht_reset(ht_iter *it);

#endif // PI_TABLE_H