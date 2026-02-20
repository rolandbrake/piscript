#ifndef PI_VALUE_H
#define PI_VALUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "string.h"
#include "pi_token.h"
#include "list.h"

typedef struct Object Object;

#define IS_NUM(val) ((val).type == VAL_NUM)
#define IS_NAN(val) ((val).type == VAL_NUM && (val).data.number == NAN)
#define IS_BOOL(val) ((val).type == VAL_BOOL)
#define IS_NIL(val) ((val).type == VAL_NIL)
#define IS_OBJ(val) ((val).type == VAL_OBJ)
#define IS_STR(val) ((val).type == VAL_OBJ && AS_OBJ(val)->type == OBJ_STRING)

#define AS_NUM(val) ((val).data.number)
#define AS_BOOL(val) ((val).data.boolean)
#define AS_OBJ(val) ((val).data.object)

#define NEW_NUM(val) ((Value){VAL_NUM, {.number = val}})           // Macro for creating a number value
#define NEW_NAN() ((Value){VAL_NUM, {.number = NAN}})              // Macro for creating a NaN value
#define NEW_BOOL(val) ((Value){VAL_BOOL, {.boolean = val}})        // Macro for creating a boolean value
#define NEW_NIL() ((Value){VAL_NIL, {.number = 0}})                // Macro for creating a nil value
#define NEW_OBJ(obj) ((Value){VAL_OBJ, {.object = (Object *)obj}}) // Macro for creating an object value

#define AS_INT(val) ((int)val.data.number)

#define VALUE_SIZE sizeof(Value)

// Enum for Value types
typedef enum
{
    VAL_NUM,
    VAL_BOOL,
    VAL_NIL,
    VAL_OBJ,
} v_type;

typedef struct Value
{
    v_type type;
    union
    {
        double number;
        bool boolean;
        Object *object;

    } data;

} Value;

typedef struct UpValue
{
    Value value;
    int index;
    struct UpValue *next;
} UpValue;

// Struct for key-value pair

bool equals(Value left, Value right);

int compare(Value left, Value right);

Value new_value(token_t token);
UpValue new_upvalue(Value value, int index);

double as_number(Value val);
bool as_bool(Value val);
char *as_string(Value val);
list_t *as_list(Value val);

bool is_numeric(Value val);

Value copy_value(Value val);
void print_value(Value val, bool is_root);

char *type_name(Value val);

#endif