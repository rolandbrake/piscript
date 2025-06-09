#include "pi_obj.h"

// Clones a PiMap object, preserving its prototype chain and key-value pairs.
Value pi_clone(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_MAP(argv[0]))
        error("[clone] expects a map as the first argument.");

    PiMap *original = AS_MAP(argv[0]);

    // Create a new empty map and mark it as an instance
    table_t *new_table = ht_create(sizeof(Value));
    Object *obj = new_map(new_table, false); // true = is_instance
    PiMap *map = (PiMap *)obj;

    // Set the prototype to the original map
    map->proto = original;

    // Copy each key-value pair into the new map
    list_t *keys = ht_keys(original->table);
    for (int i = 0; i < keys->size; i++)
    {
        char *key = string_get(keys, i);
        Value *value = (Value *)ht_get(original->table, key);
        if (value)
            ht_put(map->table, key, value);
    }

    return NEW_OBJ(map);
}

Value pi_values(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_MAP(argv[0]))
        error("[values] expects a map as the first argument.");

    PiMap *map = AS_MAP(argv[0]);
    list_t *keys = ht_keys(map->table);

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < keys->size; i++)
    {
        char *key = string_get(keys, i);
        Value *val = ht_get(map->table, key);
        if (val)
            list_add(list, val); // Copy value to the list
    }

    return NEW_OBJ(new_list(list));
}

Value pi_keys(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_MAP(argv[0]))
        error("[keys] expects a map as the first argument.");

    PiMap *map = AS_MAP(argv[0]);
    list_t *keys = ht_keys(map->table);

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < keys->size; i++)
    {
        char *key = string_get(keys, i);
        list_add(list, &NEW_OBJ(new_pistring(key)));
    }

    return NEW_OBJ(new_list(list));
}
