#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pi_table.h"
#include "pi_value.h"
#include "string.h"
#include "common.h"

// FNV-1a hash function (64-bit)
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

// Create a table for values of size `i_size`
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

    // TODO: check the size of each string inside this list
    table->_keys = list_create(sizeof(String));

    table->_last = 0;
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
    char *key_copy = strdup(key);
    if (!key_copy)
        return false;

    void *value_copy = malloc(table->i_size); // Allocate memory for the value
    if (!value_copy)
    {
        free(key_copy);
        return false;
    }
    memcpy(value_copy, value, table->i_size); // Copy the value

    table->items[index].key = key_copy;
    table->items[index].value = value_copy;
    table->items[index].hash = hash;
    table->size++;

    // Store key in the ordered list (if not already present)
    list_add(table->_keys, new_string(key_copy));

    return true;
}

bool ht_expand(table_t *table)
{
    const int new_capacity = table->capacity * 2;
    const int new_mask = new_capacity - 1;
    ht_item *new_items = calloc(new_capacity, sizeof(ht_item));
    if (!new_items)
        return false;

    // Rehash using stored hash values
    for (int i = 0; i < table->capacity; i++)
    {
        ht_item item = table->items[i];
        if (item.key != NULL)
        {
            int new_index = item.hash & new_mask;
            while (new_items[new_index].key != NULL)
                new_index = (new_index + 1) & new_mask;
            new_items[new_index] = item;
        }
    }

    free(table->items);
    table->items = new_items;
    table->capacity = new_capacity;
    return true;
}

int ht_length(table_t *table) { return table->size; }

list_t *ht_keys(table_t *table) { return table->_keys; }

void ht_free(table_t *table)
{
    for (int i = 0; i < table->capacity; i++)
    {
        if (table->items[i].key)
        {
            free(table->items[i].value);
            free(table->items[i].key); // free(NULL) is safe
        }
    }

    free(table->items);
    list_free(table->_keys);
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
    list_t *keys = table->_keys; // Use keys list

    if (it->_index >= list_size(keys))
        return false; // End of iteration

    char *key = string_get(keys, it->_index++); // Get key by insertion order
    void *value = ht_get(table, key);           // Find corresponding item

    it->key = key;
    it->value = value;
    return true;

    return false; // Should never reach here unless `ht_find` fails
}

bool ht_hasNext(ht_iter *it)
{
    return it->_index < list_size(it->_table->_keys); // Stop at actual number of keys
}

void ht_reset(ht_iter *it)
{
    it->_index = 0;
}