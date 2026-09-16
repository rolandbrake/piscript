#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "pi_object.h"
#include "common.h"

#define CREATE_OBJ(obj, type) (obj *)create_obj(sizeof(obj), type)

static Object *create_obj(size_t size, o_type type)
{
    Object *obj = (Object *)malloc(size);
    obj->type = type;
    obj->is_marked = false;
    obj->in_gcList = false;
    obj->gc_color = GC_WHITE;

    return obj;
}

/**
 * Computes the 64-bit FNV-1a hash of exactly length bytes.
 * Uses the same offset basis and prime as the hash tables.
 */
uint64_t string_hash(char *chars, size_t length)
{
    uint64_t hash = FNV_OFFSET;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)chars[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

/**
 * Creates a new PiString object from a given C-string.
 *
 * This function creates a new PiString object, which is a wrapper around
 * a C-string with additional metadata like length and hash. It calculates
 * the length and hash of the provided string and sets the initial state
 * for iteration.
 *
 * @param str The C-string to wrap in a PiString object.
 * @return A pointer to the newly created PiString object, cast as Object.
 */
Object *new_pistring(char *str)
{
    // Allocate memory for the PiString object and initialize its type
    PiString *string = CREATE_OBJ(PiString, OBJ_STRING);

    // Set the string characters to the input C-string
    string->chars = str;

    // Calculate and store the length of the string
    string->length = strlen(str);

    // Calculate and store the hash of the string
    string->hash = string_hash(str, string->length);

    // Initialize the current position for iteration
    string->current = 0;

    // Return the PiString object cast as a generic Object
    return (Object *)string;
}

/**
 * Copies a given C-string into a new PiString object.
 *
 * This function allocates a new PiString object and copies the
 * given C-string into it. It also calculates the hash of the
 * string and sets the initial state for iteration.
 *
 * @param chars The C-string to copy into the new PiString object.
 * @param length The length of the input C-string.
 * @return A pointer to the newly created PiString object.
 */
PiString *copy_pistring(char *chars, int length)
{
    PiString *string = CREATE_OBJ(PiString, OBJ_STRING);

    string->length = length;
    string->chars = malloc(length + 1);

    if (!string->chars)
        error("[copy_pistring] Memory allocation failed.");

    // Copy the input C-string into the new PiString object
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    // Calculate the hash of the string
    string->hash = string_hash(chars, length);

    // Initialize the current position for iteration
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

/**
 * Creates a new PiMap object from a given table.
 *
 * @param table The underlying table that this object wraps.
 * @param is_instance Whether this object is an instance of another object.
 * @return The newly created PiMap object.
 */
Object *new_map(table_t *table, bool is_instance)
{
    PiMap *map = CREATE_OBJ(PiMap, OBJ_MAP);

    // Store the given table in the object
    map->table = table;

    // Initialize the iterator for the object
    map->it = ht_iterator(table);

    // Store whether this object is an instance of another object
    map->is_instance = is_instance;

    // Set the prototype to NULL
    map->proto = NULL;

    return (Object *)map;
}

/**
 * Creates a new ObjFile object that represents a file stream.
 *
 * @param file The underlying FILE * that this object wraps.
 * @param filename The name of the file.
 * @param mode The mode string that was used to open the file.
 * @return The newly created ObjFile object.
 */
Object *new_file(FILE *file, char *filename, char *mode)
{
    ObjFile *f = CREATE_OBJ(ObjFile, OBJ_FILE);

    f->fp = file;
    f->filename = filename;
    f->mode = mode;
    f->closed = false;

    return (Object *)f;
}

/**
 * Creates a new ObjModel3d object from a given array of triangles and their count.
 *
 * @param triangles The array of triangles to contain in the new ObjModel3d object.
 * @param count The number of triangles in the given array.
 * @param texture The optional texture to use for the model. If NULL, no texture is used.
 * @return The newly created ObjModel3d object.
 */
ObjModel3d *new_model3d(triangle *triangles, int count, ObjImage *texture)
{
    ObjModel3d *obj = CREATE_OBJ(ObjModel3d, OBJ_MODEL3D);
    obj->triangles = triangles;
    obj->count = count;
    obj->texture = texture; // Optional texture NULL if no texture
    return obj;
}

/**
 * Creates a new ObjImage object with the given width, height, and pixel data.
 *
 * @param width The width of the image.
 * @param height The height of the image.
 * @param pixels The packed 0xAARRGGBB pixel data of the image (width*height long).
 * @return The newly created ObjImage object.
 */
ObjImage *new_image(int width, int height, uint32_t *pixels)
{
    ObjImage *obj = CREATE_OBJ(ObjImage, OBJ_IMAGE);

    // Store the given width and height in the object
    obj->width = width;
    obj->height = height;

    // Store the given pixel data in the object
    obj->pixels = pixels;

    return obj;
}

ObjSound *new_sound(Mix_Chunk *chunk)
{
    ObjSound *sound = CREATE_OBJ(ObjSound, OBJ_SOUND);
    sound->chunk = chunk;
    sound->music = NULL;
    sound->channel = -1;
    sound->pause_position = 0.0;
    sound->started_ticks = 0;
    sound->pitch = 1.0;
    sound->volume = MIX_MAX_VOLUME;
    sound->loaded = (chunk != NULL);
    sound->looping = false;
    sound->paused = false;
    sound->is_cart = false;
    sound->is_music = false;
    memset(&sound->data, 0, sizeof(Sound));
    return sound;
}

ObjSprite *new_sprite(uint16_t width, uint16_t height, uint8_t *data)
{
    ObjSprite *sprite = CREATE_OBJ(ObjSprite, OBJ_SPRITE);
    sprite->width = width;
    sprite->height = height;
    sprite->pixels = data;
    return sprite;
}

/**
 * Retrieves the value associated with a given key from a PiMap.
 *
 * This function searches for the specified key in the map's
 * underlying table. If the key exists, it returns the corresponding
 * value. Otherwise, it returns a nil value.
 *
 * @param map The map from which to retrieve the value.
 * @param key The key whose associated value is to be returned.
 * @return The value associated with the specified key, or nil if
 *         the key does not exist in the map.
 */
Value map_get(PiMap *map, Value key)
{
    char *key_str = as_string(key);
    // Attempt to retrieve the item from the hash table using the key
    void *item = ht_get(map->table, key_str);
    free(key_str);

    // Check if the item was found; if not, return nil
    if (item == NULL)
        return NEW_NIL();

    // Return the found value
    return *(Value *)item;
}

/**
 * Checks if a given key exists in a PiMap.
 *
 * This function searches for the specified key in the map's
 * underlying table. If the key exists, it returns true.
 * Otherwise, it returns false.
 *
 * @param map The map to search for the given key.
 * @param key The key to search for.
 * @return true if the key exists in the map, false otherwise.
 */
bool map_has(PiMap *map, Value key)
{
    char *key_str = as_string(key);
    bool found = ht_get(map->table, key_str) != NULL;
    free(key_str);
    return found;
}

/**
 * Sets the value associated with a given key in a PiMap.
 *
 * This function either creates a new key-value pair in the map's
 * underlying table or updates the value associated with an existing
 * key. If the key does not exist in the table, it is added. If the
 * key already exists, its associated value is updated.
 *
 * @param map The map in which to set the value.
 * @param key The key with which to associate the value.
 * @param value The value to associate with the given key.
 */
void map_set(PiMap *map, Value key, Value value)
{
    char *key_str = as_string(key);
    // Attempt to set the item in the hash table using the key
    bool updated = ht_set(map->table, key_str, &value);

    // If the key does not exist in the table, add it
    if (!updated)
        ht_put(map->table, key_str, &value);

    free(key_str);
}

/**
 * Returns the size of a PiMap.
 *
 * This function returns the number of key-value pairs in the map's
 * underlying table.
 *
 * @param map The map for which to return the size.
 * @return The number of key-value pairs in the map.
 */
int map_size(PiMap *map)
{
    return map->table->size;
}

/* Bytecode hashes retain their existing 32-bit representation. */
uint32_t code_hash(uint8_t *code)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < 16; i++)
    {
        hash ^= code[i];
        hash *= 16777619u;
    }
    return hash;
}

/**
 * Creates a new ObjCode object containing the given code.
 *
 * This function creates a new ObjCode object containing the given
 * code list and computes a hash value for the code using the
 * code_hash() function. The hash value is stored in the object for
 * quick comparison.
 *
 * @param code A pointer to the code list to be stored in the object.
 * @return The newly created ObjCode object.
 */
Object *new_code(list_t *code)
{
    ObjCode *c = CREATE_OBJ(ObjCode, OBJ_CODE);

    // Compute the hash value of the code
    c->hash = code_hash((uint8_t *)code->data);

    // Store the code list in the object
    c->data = code;

    return (Object *)c;
}
/**
 * Creates a new ObjRange object with the given start, end, and step values.
 *
 * This function allocates a new ObjRange object and initializes its
 * start, end, and step fields with the given values. The current value
 * of the range is set to the start value.
 *
 * @param start The start value of the range (inclusive).
 * @param end The end value of the range (exclusive).
 * @param step The step value of the range.
 * @return The newly created ObjRange object.
 */
Object *new_range(double start, double end, double step)
{
    PiRange *range = CREATE_OBJ(PiRange, OBJ_RANGE);

    range->start = start;
    range->end = end;
    range->step = step;

    range->current = start;

    return (Object *)range;
}

Object *new_slice(double start, double end, double step)
{
    PiSlice *slice = CREATE_OBJ(PiSlice, OBJ_SLICE);

    slice->start = start;
    slice->end = end;
    slice->step = step;

    return (Object *)slice;
}

/**
 * Resets the given iterable object to its initial state.
 *
 * This function resets the current item in the given iterable object
 * to its initial state. For ranges, this means the start value. For
 * lists and strings, this means the first element. For maps, this
 * means the first key-value pair.
 *
 * @param col The iterable object to be reset.
 */
void iter_reset(Object *col)
{
    switch (col->type)
    {
    case OBJ_RANGE:
        // Reset the current value of the range to its start value
        ((PiRange *)col)->current = ((PiRange *)col)->start;
        break;
    case OBJ_LIST:
        // Reset the current index of the list to 0
        ((PiList *)col)->current = 0;
        break;
    case OBJ_STRING:
        // Reset the current index of the string to 0
        ((PiString *)col)->current = 0;
        break;
    case OBJ_MAP:
    {
        // Reset the map's iterator to its first key-value pair
        PiMap *map = (PiMap *)col;
        ht_reset(&map->it);
        break;
    }
    default:
        // Raise an error if the object type is not iterable
        fprintf(stderr, "Object type is not iterable.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Checks if the given iterable object has more items to iterate.
 *
 * This function takes an iterable object and checks if there are more
 * items to iterate. If the object is a range, it checks if the current
 * value is less than (or greater than, if the step is negative) the end
 * value. If the object is a list or string, it checks if the current index
 * is less than the length of the list or string. If the object is a map, it
 * checks if there are more key-value pairs to iterate.
 *
 * @param col The iterable object to be checked.
 * @return true if the object has more items to iterate, false otherwise.
 */
bool iter_hasNext(Object *col)
{
    o_type type = col->type;
    if (type == OBJ_LIST)
    {
        PiList *list = (PiList *)col;
        // Check if the current index is less than the length of the list
        return list->current < LIST_SIZE(list->items);
    }
    else if (type == OBJ_STRING)
    {
        PiString *str = (PiString *)col;
        // Check if the current index is less than the length of the string
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
        // Check if there are more key-value pairs to iterate
        return ht_hasNext(&map->it);
    }
    return false;
}

/**
 * Retrieves the next item from the iterable object.
 *
 * This function takes an iterable object and returns the next item in the
 * iteration. If the object is a list or string, it returns the item at the
 * current index. If the object is a range, it returns the current value and
 * increments the current value by the step. If the object is a map, it returns
 * the value associated with the current key.
 *
 * @param col The iterable object to retrieve the next item from.
 * @return The next item in the iteration.
 */
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

/**
 * @brief Check if an object is iterable.
 *
 * This function determines whether a given object can be iterated over.
 * Supported iterable types include lists, strings, ranges, and maps.
 *
 * @param obj The object to check for iterability.
 * @return true if the object is iterable, false otherwise.
 */
bool is_iterable(Object *obj)
{
    if (!obj)
        return false; // Return false if the object is null

    switch (obj->type)
    {
    case OBJ_LIST:
    case OBJ_STRING:
    case OBJ_RANGE:
    case OBJ_MAP:
        return true; // Return true for iterable types
    default:
        return false; // Return false for non-iterable types
    }
}

/* Element indices support one negative offset, never modulo wrapping. */
int get_index(int index, int length)
{
    if (length <= 0)
        error("Index out of range.");
    if (index < 0)
        index += length;
    if (index < 0 || index >= length)
        error("Index out of range.");
    return index;
}

static int slice_index(double index, int length, int step)
{
    if (isnan(index))
        error("Slice bounds cannot be NaN.");
    index = trunc(index);
    if (index < 0)
        index += length;
    int lower = step > 0 ? 0 : -1;
    int upper = step > 0 ? length : length - 1;
    if (index < lower)
        return lower;
    if (index > upper)
        return upper;
    return (int)index;
}

Value get_slice(Object *sequence, double start, double end, double step)
{
    if (!isfinite(step) || step < INT_MIN || step > INT_MAX)
        error("Slice step must be a finite integer-sized number.");
    int stride = (int)step;
    if (stride == 0)
        error("Slice step cannot be zero after integer conversion.");

    int size;
    if (sequence->type == OBJ_LIST)
        size = LIST_SIZE(((PiList *)sequence)->items);
    else if (sequence->type == OBJ_STRING)
        size = ((PiString *)sequence)->length;
    else
    {
        error("Invalid sequence type.");
        return NEW_NIL();
    }

    // The parser represents omitted bounds with infinity. An explicit -1
    // must still resolve to the last item, not the omitted reverse stop.
    int first = isinf(start) ? (stride > 0 ? 0 : size - 1)
                             : slice_index(start, size, stride);
    int stop = isinf(end) ? (stride > 0 ? size : -1)
                          : slice_index(end, size, stride);
    // Widen arithmetic so even INT_MIN / INT_MAX strides cannot overflow.
    int64_t distance = stride > 0 ? (int64_t)stop - first : (int64_t)first - stop;
    int64_t magnitude = stride > 0 ? (int64_t)stride : -(int64_t)stride;
    int count = distance > 0 ? (int)((distance - 1) / magnitude + 1) : 0;

    if (sequence->type == OBJ_LIST)
    {
        list_t *src = ((PiList *)sequence)->items;
        list_t *dst = list_create(sizeof(Value));
        int64_t index = first;
        for (int j = 0; j < count; j++, index += stride)
            list_add(dst, list_getAt(src, (int)index));
        return NEW_OBJ(new_list(dst));
    }

    PiString *str = (PiString *)sequence;
    char *buf = malloc((size_t)count + 1);
    if (!buf)
        error("[get_slice] Memory allocation failed.");
    int64_t index = first;
    for (int j = 0; j < count; j++, index += stride)
        buf[j] = str->chars[index];
    buf[count] = '\0';
    return NEW_OBJ(new_pistring(buf));
}

void free_sound(ObjSound *sound)
{
    if (sound && sound->chunk)
        Mix_FreeChunk(sound->chunk);
    if (sound && sound->music)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(sound->music);
    }
}
