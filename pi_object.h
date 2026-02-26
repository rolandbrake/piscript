#ifndef PI_OBJECT_H
#define PI_OBJECT_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "pi_value.h"
#include "list.h"
#include "pi_table.h"
#include "common.h"
#include "audio.h"

#define OBJ_TYPE(o) (AS_OBJ(o)->type)
#define IS_OBJ_TYPE(o, _type) (IS_OBJ(o) && AS_OBJ(o)->type == _type)

#define IS_STRING(o) IS_OBJ_TYPE(o, OBJ_STRING)
#define IS_LIST(o) IS_OBJ_TYPE(o, OBJ_LIST)
#define IS_NUM_LIST(o) (IS_LIST(o) && AS_LIST(o)->is_numeric)
#define IS_MAP(o) IS_OBJ_TYPE(o, OBJ_MAP)
#define IS_FUN(o) IS_OBJ_TYPE(o, OBJ_FUN)
#define IS_RANGE(o) IS_OBJ_TYPE(o, OBJ_RANGE)
#define IS_MODEL(o) IS_OBJ_TYPE(o, OBJ_MODEL3D)
#define IS_IMAGE(o) IS_OBJ_TYPE(o, OBJ_IMAGE)
#define IS_SPRITE(o) IS_OBJ_TYPE(o, OBJ_SPRITE)

#define IS_COLLECTION(o) (IS_LIST(o) || IS_MAP(o) || IS_STRING(o))

#define IS_SEQUENCE(o) (IS_LIST(o) || IS_STRING(o))

#define AS_STRING(o) ((PiString *)AS_OBJ(o))
#define AS_LIST(o) ((PiList *)AS_OBJ(o))
#define AS_MAP(o) ((PiMap *)AS_OBJ(o))
#define AS_RANGE(o) ((PiRange *)AS_OBJ(o))
#define AS_FUN(o) ((Function *)AS_OBJ(o))
#define AS_CODE(o) ((ObjCode *)AS_OBJ(o))
#define AS_FILE(o) ((ObjFile *)AS_OBJ(o))
#define AS_IMAGE(o) ((ObjImage *)AS_OBJ(o))
#define AS_SPRITE(o) ((ObjSprite *)AS_OBJ(o))

#define AS_CSTRING(o) AS_STRING(o)->chars

#define AS_CLIST(o) AS_LIST(o)->items
#define AS_CMAP(o) AS_MAP(o)->table

#define PISTR_SIZE(o) AS_STRING(o)->length
#define PIMAP_SIZE(o) AS_MAP(o)->table->size
#define PILIST_SIZE(o) AS_LIST(o)->items->size

#define COL_LENGTH(o) (IS_LIST(o) ? PILIST_SIZE(o) : PISTR_SIZE(o))

#define PILIST_GETAT(o, i, t) (*(t *)list_getAt(AS_CLIST(o), i))

typedef enum
{
    OBJ_STRING,
    OBJ_LIST,
    OBJ_MAP,
    OBJ_RANGE,
    OBJ_FUN,
    OBJ_CODE,
    OBJ_FILE,
    OBJ_IMAGE,
    OBJ_SPRITE,
    OBJ_MODEL3D,
    OBJ_SOUND
} o_type;

typedef enum
{
    GC_WHITE, // Unmarked, potentially unreachable
    GC_GRAY,  // Marked but children not yet processed
    GC_BLACK  // Marked and all children processed
} GCColor;

struct Object
{
    o_type type;
    bool is_marked; // Flag to indicate if the object is marked for garbage collection
    bool in_gcList; // Flag to indicate if the object is in the GC list

    GCColor gc_color;

    struct Object *next;
};

typedef struct
{
    Object object;
    char *chars;
    size_t length;
    uint32_t hash;

    int current;
} PiString;

typedef struct
{
    Object object;
    double start;
    double end;
    double step;

    double current; // Iterator state: current value in the range
} PiRange;

typedef struct
{
    Object object;
    list_t *items;

    int current;     // Iterator state
    bool is_numeric; // Flag to indicate if the list contains only double values
    bool is_matrix;  // Flag to indicate if the list is a 2D matrix

    // Matrix dimensions
    int rows;
    int cols;

} PiList;

typedef struct PiMap
{
    Object object;
    table_t *table;
    bool is_instance;

    struct PiMap *proto; // Prototype map for inheritance and method lookup

    // int current; // Iterator state
    ht_iter it;
} PiMap;

typedef struct
{
    Object object;
    list_t *data;

    uint32_t hash;
} ObjCode;

typedef struct
{
    Object object;
    FILE *fp;
    bool closed;
    char *mode;
    char *filename;
} ObjFile;

typedef struct
{
    Object object;   // Required for GC/type tagging
    int width;       // Width of the image (should be 128 usually)
    int height;      // Height of the image (should be 128 usually)
    uint8_t *pixels; // A flat array of width*height values (0–31)
    uint8_t *alpha;  // A flat array of width*height values (0–255)
} ObjImage;

typedef struct
{
    Object object;
    triangle *triangles;
    int count;         // Number of triangles
    ObjImage *texture; // Optional texture
} ObjModel3d;

typedef struct
{
    Object object;
    Mix_Chunk *chunk;
    int channel;
    bool loaded;
    bool looping;
    bool is_cart; // Flag to indicate if the sound was loaded from the cartridge
    Sound data;   // Sound data from the cartridge if is_cart is true
} ObjSound;

typedef struct
{
    Object object;
    uint16_t width;
    uint16_t height;
    uint8_t *data;
} ObjSprite;

uint32_t string_hash(char *chars, size_t length);
Object *new_pistring(char *str);
PiString *copy_pistring(char *chars, int length);

Object *new_list(list_t *items);

Object *new_map(table_t *table, bool is_instance);

Object *new_file(FILE *file, char *filename, char *mode);
ObjModel3d *new_model3d(triangle *triangles, int count, ObjImage *texture);
ObjImage *new_image(int width, int height, uint8_t *pixels, uint8_t *alpha);
ObjSound *new_sound(Mix_Chunk *chunk);
ObjSprite *new_sprite(uint16_t width, uint16_t height, uint8_t *data);

Value map_get(PiMap *map, Value key);
void map_set(PiMap *map, Value key, Value value);
bool map_has(PiMap *map, Value key);

int map_size(PiMap *map);

Object *new_range(double start, double end, double step);

uint32_t code_hash(uint8_t *code);
Object *new_code(list_t *code);

void iter_reset(Object *col);
bool iter_hasNext(Object *col);
Value iter_next(Object *col);
bool is_iterable(Object *obj);
int get_index(int index, int length);
Value get_slice(Object *sequence, double start, double end, double step);

void free_sound(ObjSound *sound);

#endif
