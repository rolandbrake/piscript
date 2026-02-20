#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pi_table.h"
#include "pi_value.h"
#include "string.h"
#include "common.h"

/**
 * FNV-1a hash function
 *
 * The FNV-1a hash is based on an algorithm originally developed by Landon Curt Noll.
 * See http://www.isthe.com/chongo/tech/comp/fnv/ for more information.
 *
 * @param key The string to hash
 * @return The hash value
 */
static inline uint64_t FNV_1a(const char *key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char *p = key; *p; p++)
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

/**
 * Creates a new table with the specified item size and initial capacity.
 *
 * This function allocates memory for the table structure and initializes its
 * members. It sets the initial capacity to a fixed constant and allocates
 * memory for the items array to store the key-value pairs and the _keys array
 * to store the key pointers in insertion order.
 *
 * @param i_size The size of each item to be stored in the table.
 * @return A pointer to the newly created table.
 */
table_t *ht_create(size_t i_size)
{
    table_t *table = malloc(sizeof(table_t));
    if (!table)
        return NULL;

    table->size = 0;
    table->capacity = INIT_CAP;
    table->i_size = i_size; // Store value size
    table->items = calloc(table->capacity, sizeof(ht_item));
    if (!table->items)
    {
        free(table);
        return NULL;
    }

    table->_keys = calloc(INIT_CAP, sizeof(char *)); // allocate space for key pointers
    table->_last = 0;                                // Initialize _last to the first index

    return table;
}

void *ht_get(table_t *table, const char *key)
{

    uint64_t hash = FNV_1a(key);
    int mask = table->capacity - 1;
    int index = hash & mask;

    while (table->items[index].key != NULL)
    {
        if (table->items[index].hash == hash &&
            strcmp(table->items[index].key, key) == 0)
            return table->items[index].value;

        index = (index + 1) & mask;
    }

    return NULL;
}

bool ht_set(table_t *table, const char *key, const void *value)
{
    uint64_t hash = FNV_1a(key);
    int mask = table->capacity - 1;
    int index = hash & mask;

    while (table->items[index].key != NULL)
    {
        if (table->items[index].hash == hash &&
            strcmp(table->items[index].key, key) == 0)
        {
            // Update existing value
            memcpy(table->items[index].value, value, table->i_size);
            return true;
        }
        index = (index + 1) & mask;
    }

    return false; // Key not found, no update
}

bool ht_put(table_t *table, const char *key, const void *value)
{

    // Check if we need to expand (load factor > 0.75)
    if ((table->size + 1) * 4 > table->capacity * 3)
        if (!ht_expand(table))
            return false;

    uint64_t hash = FNV_1a(key);
    int mask = table->capacity - 1;
    int index = hash & mask;

    // Check for existing key
    while (table->items[index].key != NULL)
    {
        if (table->items[index].hash == hash &&
            strcmp(table->items[index].key, key) == 0)
        {
            // Update existing value
            memcpy(table->items[index].value, value, table->i_size);
            return true;
        }
        index = (index + 1) & mask;
    }

    if (index > table->_last)
        table->_last = index;

    // Insert new key-value pair
    char *_key = strdup(key);

    void *_value = malloc(table->i_size); // Allocate memory for the value
    memcpy(_value, value, table->i_size); // Copy the value

    table->items[index].key = _key;
    table->items[index].value = _value;
    table->items[index].hash = hash;

    // Store key in _keys array in insertion order
    table->_keys[table->size] = _key;

    table->size++;

    return true;
}

/**
 * Expand the table to double its capacity.
 *
 * @param table The table to be expanded.
 * @return true if the expansion was successful, false otherwise.
 */
bool ht_expand(table_t *table)
{
    // Calculate the new capacity and mask
    const int new_cap = table->capacity * 2;
    const int new_mask = new_cap - 1;

    // Allocate memory for the new items array
    ht_item *new_items = calloc(new_cap, sizeof(ht_item));
    if (!new_items)
        return false;

    // Rehash using stored hash values
    for (int i = 0; i < table->capacity; i++)
    {
        ht_item item = table->items[i];
        if (item.key != NULL)
        {
            // Find the new index for the item
            int index = item.hash & new_mask;
            while (new_items[index].key != NULL)
                index = (index + 1) & new_mask;
            new_items[index] = item;
        }
    }

    // Free the old items array and update the table
    free(table->items);
    table->items = new_items;

    // ✅ Reallocate _keys to match new capacity
    char **new_keys = realloc(table->_keys, new_cap * sizeof(char *));
    if (!new_keys)
        return false;
    table->_keys = new_keys;

    // Update capacity
    table->capacity = new_cap;

    return true;
}

int ht_length(table_t *table) { return table->size; }

int ht_last(table_t *table) { return table->_last; }

char **ht_keys(table_t *table) { return table->_keys; }

void ht_free(table_t *table)
{
    if (!table)
        return;

    for (int i = 0; i < table->capacity; i++)
    {
        if (table->items[i].key)
        {
            free(table->items[i].key);
            free(table->items[i].value);
        }
    }

    if (table->_keys)
        free(table->_keys);

    free(table->items);
    free(table);
}

// Iterator functions
// TODO: return iterator pointer instead of value!
ht_iter ht_iterator(table_t *table)
{
    ht_iter it = {0};
    it._table = table;
    it._index = 0;
    return it;
}

bool ht_next(ht_iter *it)
{
    table_t *table = it->_table;

    if (it->_index >= table->size)
        return false; // End of iteration

    char *key = table->_keys[it->_index++]; // Get key by insertion order
    void *value = ht_get(table, key);       // Find corresponding value

    it->key = key;
    it->value = value;
    return true;
}

bool ht_hasNext(ht_iter *it)
{
    return it->_index < it->_table->size; // Stop at actual number of keys
}

void ht_reset(ht_iter *it)
{
    it->_index = 0;
}