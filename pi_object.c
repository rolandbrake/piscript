#include <math.h>
#include "pi_object.h"
#include "common.h"

#define CREATE_OBJ(obj, type) (obj *)create_obj(sizeof(obj), type)

static Object *create_obj(size_t size, o_type type)
{
    Object *obj = (Object *)malloc(size);
    obj->type = type;
    obj->is_marked = false;

    return obj;
}

uint32_t string_hash(char *chars, size_t length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)chars[i];
        hash *= 16777619;
    }
    return hash;
}

Object *new_pistring(char *str)
{
    PiString *string = CREATE_OBJ(PiString, OBJ_STRING);
    string->chars = str;
    string->length = strlen(str);
    string->hash = string_hash(str, string->length);
    string->current = 0;
    return (Object *)string;
}

PiString *copy_pistring(char *chars, int length)
{
    PiString *string = CREATE_OBJ(PiString, OBJ_STRING);

    string->length = length;
    string->chars = malloc(length + 1);

    if (!string->chars)
        error("[copy_pistring] Memory allocation failed.");

    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    string->hash = string_hash(chars, length);
    string->current = 0;
    return string;
}

/**
 * Creates a new PiList object containing the given list of items.
 *
 * @param items The list of items to contain in the new PiList object.
 * @return The newly created PiList object.
 */
Object *new_list(list_t *items)
{
    PiList *list = CREATE_OBJ(PiList, OBJ_LIST);
    list->items = items;
    list->current = 0;
    list->is_numeric = false;
    list->cols = -1;
    list->rows = -1;
    return (Object *)list;
}

Object *new_map(table_t *table, bool is_instance)
{
    PiMap *map = CREATE_OBJ(PiMap, OBJ_MAP);
    map->table = table;
    map->it = ht_iterator(table);

    map->is_instance = is_instance;
    return (Object *)map;
}

Value map_get(PiMap *map, Value key)
{
    void *item = ht_get(map->table, as_string(key));

    if (item == NULL)
        return NEW_NIL();
    else
        return *(Value *)item;
}

bool map_has(PiMap *map, Value key)
{
    return ht_get(map->table, as_string(key)) != NULL;
}

void map_set(PiMap *map, Value key, Value value)
{
    bool updated = ht_set(map->table, as_string(key), &value);
    if (!updated)
        ht_put(map->table, as_string(key), &value);
}

int map_size(PiMap *map)
{
    return map->table->size;
}

uint32_t code_hash(uint8_t *code)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < 16; i++)
    {
        hash ^= code[i];
        hash *= 16777619;
    }
    return hash;
}

Object *new_code(list_t *code)
{
    PiCode *c = CREATE_OBJ(PiCode, OBJ_CODE);
    c->hash = code_hash((uint8_t *)code->data);
    c->data = code;
    return (Object *)c;
}
Object *new_range(double start, double end, double step)
{
    PiRange *range = CREATE_OBJ(PiRange, OBJ_RANGE);

    range->start = start;
    range->end = end;
    range->step = step;

    range->current = start;

    return (Object *)range;
}

void iter_reset(Object *col)
{
    switch (col->type)
    {
    case OBJ_RANGE:
        ((PiRange *)col)->current = ((PiRange *)col)->start;
        break;
    case OBJ_LIST:
        ((PiList *)col)->current = 0;
        break;
    case OBJ_STRING:
        ((PiString *)col)->current = 0;
        break;
    case OBJ_MAP:
    {
        PiMap *map = (PiMap *)col;
        ht_reset(&map->it);
        break;
    }
    default:
        fprintf(stderr, "Object type is not iterable.\n");
        exit(EXIT_FAILURE);
    }
}

bool iter_hasNext(Object *col)
{
    o_type type = col->type;
    if (type == OBJ_LIST)
    {
        PiList *list = (PiList *)col;
        return list->current < LIST_SIZE(list->items);
    }
    else if (type == OBJ_STRING)
    {
        PiString *str = (PiString *)col;
        return str->current < str->length;
    }
    else if (type == OBJ_RANGE)
    {
        PiRange *range = (PiRange *)col;
        // If step is positive, check if current < end
        // If step is negative, check if current > end
        return (range->step > 0) ? (range->current < range->end) : (range->current > range->end);
    }
    else if (type == OBJ_MAP)
    {
        PiMap *map = (PiMap *)col;
        return ht_hasNext(&map->it);
    }
    return false;
}

Value iter_next(Object *col)
{
    o_type type = col->type;
    if (type == OBJ_LIST)
    {
        PiList *list = (PiList *)col;
        Value value = *(Value *)list_getAt(list->items, list->current);
        list->current++;
        return value;
    }
    else if (type == OBJ_STRING)
    {
        PiString *str = (PiString *)col;
        char *_chars = malloc(2); // 1 char + null terminator
        _chars[0] = str->chars[str->current];
        _chars[1] = '\0';
        Value value = NEW_OBJ(new_pistring(_chars));
        str->current++;
        return value;
    }
    else if (type == OBJ_RANGE)
    {
        PiRange *range = (PiRange *)col;
        Value value = NEW_NUM(range->current);
        range->current += range->step;
        return value;
    }
    else if (type == OBJ_MAP)
    {
        PiMap *map = (PiMap *)col;
        ht_next(&map->it);
        return *(Value *)map->it.value;
    }

    fprintf(stderr, "Invalid col type for iteration.\n");
    exit(EXIT_FAILURE);
}

bool is_iterable(Object *obj)
{
    if (!obj)
        return false;

    switch (obj->type)
    {
    case OBJ_LIST:
    case OBJ_STRING:
    case OBJ_RANGE:
    case OBJ_MAP:
        return true;
    default:
        return false;
    }
}

int get_index(int index, int length)
{
    if (length == 0)
        return 0;
    int _index = index % length;
    if (_index < 0) // Handle negative indices
        _index += length;
    return _index;
}

// Value get_slice(Object *sequance, int start, int end, int step)
// {
//     int size = 0; // Size of the sequence (list or string)
//     int sign = (step > 0) ? 1 : -1;

//     if (sequance->type == OBJ_LIST)
//     {
//         PiList *list = (PiList *)sequance;
//         size = LIST_SIZE(list->items);

//         // Adjust indices
//         start = get_index(start, size);
//         end = (end == INFINITY) ? size - 1 : get_index(end, size);

//         // Create the sliced list
//         list_t *s_list = list_create(sizeof(Value));
//         while (sign * (end - start) > 0)
//         {
//             Value *item = (Value *)list_getAt(list->items, start);
//             list_add(s_list, item); // Add the item to the sublist
//             start += step;
//         }

//         return NEW_OBJ(new_list(s_list));
//     }
//     else if (sequance->type == OBJ_STRING)
//     {
//         PiString *str = (PiString *)sequance;
//         size = str->length;

//         // Adjust indices
//         start = get_index(start, size);
//         end = (end == INFINITY) ? size - 1 : get_index(end, size);

//         // Create the sliced string
//         char *s_str = malloc(end - start + 1);
//         for (int i = start; i < end; i += step)
//             s_str[i - start] = str->chars[i];

//         s_str[end - start + 1] = '\0';

//         return NEW_OBJ(new_pistring(s_str));
//     }

//     fprintf(stderr, "Invalid sequance type.\n");
//     exit(EXIT_FAILURE);
// }

Value get_slice(Object *sequence, double start, double end, double step)
{
    int size = 0; // Size of the sequence (list or string)
    int sign = (step > 0) ? 1 : -1;
    int _start, _end, _step;

    // Convert step to integer (must be non-zero)
    if (step == 0)
    {
        fprintf(stderr, "Slice step cannot be zero.\n");
        exit(EXIT_FAILURE);
    }
    _step = (int)step;

    if (sequence->type == OBJ_LIST)
    {
        PiList *list = (PiList *)sequence;
        size = LIST_SIZE(list->items);

        // Handle infinity values and convert to integers
        if (isinf(start))
            _start = (sign > 0) ? size : -1;
        else
            _start = get_index((int)start, size);

        if (isinf(end))
            _end = (sign > 0) ? size : -1;
        else
            _end = get_index((int)end, size);

        // Create the sliced list
        list_t *s_list = list_create(sizeof(Value));
        while (sign * (_end - _start) > 0)
        {
            Value *item = (Value *)list_getAt(list->items, _start);
            list_add(s_list, item); // Add the item to the sublist
            _start += _step;
        }

        return NEW_OBJ(new_list(s_list));
    }
    else if (sequence->type == OBJ_STRING)
    {
        PiString *str = (PiString *)sequence;
        size = str->length;

        // Handle infinity values and convert to integers
        if (isinf(start))
            _start = (sign > 0) ? size : -1;
        else
            _start = get_index((int)start, size);

        if (isinf(end))
            _end = (sign > 0) ? size : -1;
        else
            _end = get_index((int)end, size);

        // Create the sliced string
        char *s_str = malloc(abs(_end - _start) + 1);
        int i, j = 0;
        for (i = _start; sign * (_end - i) > 0; i += _step)
            s_str[j++] = str->chars[i];
        s_str[j] = '\0';

        return NEW_OBJ(new_pistring(s_str));
    }

    fprintf(stderr, "Invalid sequence type.\n");
    exit(EXIT_FAILURE);
}
