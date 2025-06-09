#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "pi_value.h"
#include "pi_object.h"
#include "pi_func.h"

/**
 * Checks if two values are equal.
 *
 * The comparison for numbers is a tolerance check, as exact floating-point
 * comparisons are not always reliable.
 *
 * The comparison for booleans is a direct comparison.
 *
 * The comparison for NIL is a special case, as all NIL values are considered
 * equal.
 *
 * The comparison for objects is a recursive comparison of their contents.
 *
 * @param left The first value to compare.
 * @param right The second value to compare.
 * @return true if the values are equal, false if they are not.
 */

bool equals(Value left, Value right)
{
    // If the types are different, they can't be equal.
    if (left.type != right.type)
        return false;

    switch (left.type)
    {
    case VAL_NUM:
        // Use a tolerance for floating-point comparisons.
        return fabs(left.data.number - right.data.number) < 1e-9;

    case VAL_BOOL:
        // Direct comparison for booleans.
        return left.data.boolean == right.data.boolean;

    case VAL_NIL:
        // All NIL values are considered equal.
        return true;

    case VAL_OBJ:
        if (left.data.object->type != right.data.object->type)
            return false;

        switch (left.data.object->type)
        {
        case OBJ_STRING:
        {
            PiString *a = (PiString *)left.data.object;
            PiString *b = (PiString *)right.data.object;
            if (a->length != b->length)
                return false;
            return strcmp(a->chars, b->chars) == 0;
        }

        case OBJ_LIST:
        {
            PiList *a = AS_LIST(left);
            PiList *b = AS_LIST(right);

            if (LIST_SIZE(a->items) != LIST_SIZE(b->items))
                return false; // Different sizes mean the lists cannot be equal

            for (size_t i = 0; i < LIST_SIZE(a->items); i++)
            {
                Value item_a = *(Value *)list_getAt(a->items, i);
                Value item_b = *(Value *)list_getAt(b->items, i);

                if (!equals(item_a, item_b))
                    return false; // Found a mismatch
            }
            return true; // All elements are equal
        }

        default:
            // For unsupported object types, fall back to pointer comparison.
            return left.data.object == right.data.object;
        }

    default:
        // Handle unexpected or unsupported types.
        return false;
    }
}

/**
 * Compares two values.
 *
 * @param left the first value to compare
 * @param right the second value to compare
 *
 * @return a negative value if left is less than right,
 *         zero if left is equal to right,
 *         a positive value if left is greater than right
 */

int compare(Value left, Value right)
{
    if (left.type != right.type)
    {
        // Coerce right to match left's type
        switch (left.type)
        {
        case VAL_NUM:
        {
            double l_num = AS_NUM(left);
            double r_num = AS_NUM(right);
            if (l_num < r_num)
                return -1;
            else if (l_num > r_num)
                return 1;
            else
                return 0;
        }

        case VAL_BOOL:
            return as_bool(left) - as_bool(right);

        case VAL_OBJ:
            if (OBJ_TYPE(left) == OBJ_STRING)
            {
                char *l_str = as_string(left);
                char *r_str = as_string(right);
                int result = strcmp(l_str, r_str);
                free(l_str);
                free(r_str);
                return result;
            }
            break;

        default:
            error("Unsupported type coercion for comparison");
        }
        error("Unsupported type coercion for comparison");
    }

    // If types match, compare normally
    switch (left.type)
    {
    case VAL_NUM:
        if (fabs(left.data.number - right.data.number) < 1e-9)
            return 0;
        return (left.data.number > right.data.number) ? 1 : -1;

    case VAL_BOOL:
        return (int)left.data.boolean - (int)right.data.boolean;

    case VAL_NIL:
        return 0;

    case VAL_OBJ:
        if (OBJ_TYPE(left) == OBJ_STRING)
        {
            PiString *l_str = AS_STRING(left);
            PiString *r_str = AS_STRING(right);
            return strcmp(l_str->chars, r_str->chars);
        }
        else if (OBJ_TYPE(left) == OBJ_LIST)
        {
            PiList *l_list = AS_LIST(left);
            PiList *r_list = AS_LIST(right);

            size_t l_size = LIST_SIZE(l_list->items);
            size_t r_size = LIST_SIZE(r_list->items);
            size_t min_size = (l_size < r_size) ? l_size : r_size;

            for (size_t i = 0; i < min_size; i++)
            {
                Value *l_item = list_getAt(l_list->items, i);
                Value *r_item = list_getAt(r_list->items, i);
                int cmp = compare(*l_item, *r_item);
                if (cmp != 0)
                    return cmp;
            }

            return (l_size > r_size) ? 1 : (l_size < r_size) ? -1
                                                             : 0;
        }
        else
            error("Unsupported object type for comparison");

    default:
        error("Unsupported type for comparison");
    }
}

Value new_value(token_t token)
{
    // printf("token value: %d\n", tk_double(token));
    Value val;
    switch (token.type)
    {
    case TK_NUM:
        val.type = VAL_NUM;
        val.data.number = tk_double(token);
        break;
    case TK_STR:
    case TK_ID:
        val = NEW_OBJ(new_pistring(tk_string(token)));
        break;
    case TK_TRUE:
    case TK_FALSE:
        val.type = VAL_BOOL;
        val.data.boolean = tk_bool(token);
        break;
    case TK_NIL:
        val.type = VAL_NIL;
        break;
    default:
        // Handle unexpected token type
        // TODO: Add error handling for unexpected token types
        fprintf(stderr, "Unexpected token value: %s\n", tk_string(token));
        exit(EXIT_FAILURE);
    }

    return val;
}

double as_number(Value val)
{
    switch (val.type)
    {
    case VAL_NUM:
        return val.data.number;
    case VAL_BOOL:
        return val.data.boolean ? 1.0 : 0.0;
    case VAL_NIL:
        return 0.0;
    case VAL_OBJ:
        if (AS_OBJ(val)->type == OBJ_STRING)
        {
            char *endptr;
            PiString *str = AS_STRING(val);
            double result = strtod(str->chars, &endptr);

            // Check if the entire string was successfully converted
            if (endptr == str->chars)
                error("Error: String '%s' cannot be converted to a number.", str->chars);

            return result;
        }
        // Fall through to default if object type is unsupported
        break;
    default:
        error("Cannot convert %s to a number", type_name(val));
    }

    return 0.0;
}

bool as_bool(Value val)
{
    switch (val.type)
    {
    case VAL_BOOL:
        return val.data.boolean;
    case VAL_NUM:
        return val.data.number != 0.0;
    case VAL_NIL:
        return false;
    case VAL_OBJ:
        switch (AS_OBJ(val)->type)
        {
        case OBJ_STRING:
            // A string is considered true if it is non-empty
            return AS_STRING(val)->length > 0;

        case OBJ_LIST:
            // A list is considered true if it has items
            return LIST_SIZE(AS_LIST(val)->items) > 0;

        case OBJ_MAP:
            // A map is considered true if it has key-value pairs
            return ht_length(AS_MAP(val)->table) > 0;

        case OBJ_RANGE:
            // A range is considered true if it has start and end values
            return AS_RANGE(val)->start != AS_RANGE(val)->end;

        default:
            // Other object types could default to true or false based on your language's semantics
            return true;
        }
    default:
        fprintf(stderr, "Expected a boolean, but got %s\n", type_name(val));
        exit(EXIT_FAILURE);
    }
}

char *as_string(Value val)
{
    switch (val.type)
    {
    case VAL_NUM:
    {
        char *num = (char *)malloc(32); // Allocate space for number-to-string conversion

        if (isnan(val.data.number))
            snprintf(num, 32, "NAN"); // Handle NaN case
        else if (val.data.number == INFINITY || val.data.number == -INFINITY)
            snprintf(num, 32, "%s", val.data.number == INFINITY ? "INF" : "-INF"); // Convert infinity to string
        else
            snprintf(num, 32, "%g", val.data.number); // Convert number to string
        return num;
    }
    case VAL_BOOL:
        return val.data.boolean ? "true" : "false";
    case VAL_NIL:
        return strdup("nil");
    case VAL_OBJ:
    {
        switch (AS_OBJ(val)->type)
        {
        case OBJ_STRING:
        {
            char *str = AS_STRING(val)->chars;
            return strdup(str); // Create a copy
        }
        case OBJ_LIST:
        {
            list_t *list = as_list(val);
            size_t buffer_size = 2; // Start with "[]"
            char *result = strdup("[");

            int size = list->size;
            for (size_t i = 0; i < size; i++)
            {
                if (i > 0)
                {
                    buffer_size += 2; // For ", "
                    result = realloc(result, buffer_size);
                    strcat(result, ", ");
                }

                char *item = as_string(*(Value *)list_getAt(list, i));
                buffer_size += strlen(item);
                result = realloc(result, buffer_size);
                strcat(result, item);
                // free(item);
            }

            buffer_size++; // For the closing "]"
            result = realloc(result, buffer_size);
            strcat(result, "]");

            return result;
        }

        case OBJ_MAP:
        {
            PiMap *map = AS_MAP(val);
            list_t *keys = map->table->_keys;
            int size = list_size(keys);
            if (size == 0)
                return strdup("{}");

            size_t buffer_size = 2; // Start with "{}"
            char *result = strdup("{");

            for (int i = 0; i < size; i++)
            {
                if (i > 0)
                {
                    buffer_size += 2;
                    result = realloc(result, buffer_size);
                    strcat(result, ", ");
                }

                char *key = string_get(keys, i);
                char *value = as_string(*(Value *)ht_get(map->table, key));

                buffer_size += strlen(key) + strlen(": ") + strlen(value);
                result = realloc(result, buffer_size);
                strcat(result, key);
                strcat(result, ": ");
                strcat(result, value);

                free(value);
            }

            buffer_size++;
            result = realloc(result, buffer_size);
            strcat(result, "}");

            return result;
        }
        case OBJ_FUN:
        {
            Function *fun = AS_FUN(val);

            char *result = (char *)malloc(128);
            sprintf(result, "<%s: %p>", fun->name, (void *)fun);
            return result;
        }
        case OBJ_RANGE:
        case OBJ_CODE:
            break;
        }
    }
    default:
        break;
    }

    fprintf(stderr, "Cannot cast value to string\n");
    exit(EXIT_FAILURE);
}

list_t *as_list(Value val)
{
    if (val.type == VAL_OBJ && OBJ_TYPE(val) == OBJ_LIST)
        return AS_LIST(val)->items;
    fprintf(stderr, "Expected a list, but got %s\n", type_name(val));
    exit(EXIT_FAILURE);
}
bool is_numeric(Value val)
{

    if (val.type == VAL_NUM || val.type == VAL_BOOL || val.type == VAL_NIL)
        // If the value is already a number, boolean, or nil, it is considered numeric
        return true;

    if (val.type == VAL_OBJ && OBJ_TYPE(val) == OBJ_STRING)
    {
        // Try parsing the string as a number
        char *str_value = AS_STRING(val)->chars;
        char *end_ptr;
        strtod(str_value, &end_ptr); // Convert the string to a double

        // If the conversion succeeds and consumes the whole string, it's numeric
        return *end_ptr == '\0';
    }

    return false; // All other types are not numeric
}

Value copy_value(Value val)
{
    Value copy;

    switch (val.type)
    {
    case VAL_NUM:
    case VAL_BOOL:
    case VAL_NIL:
        copy = val;
        break;

    case VAL_OBJ:
    {
        Object *obj = AS_OBJ(val);
        copy.type = VAL_OBJ;
        switch (obj->type)
        {
        case OBJ_STRING:
        {
            // Deep copy string
            PiString *original = (PiString *)obj;
            PiString *str = malloc(sizeof(PiString));

            str->object.type = OBJ_STRING;
            str->length = original->length;

            str->chars = malloc(str->length + 1);
            strcpy(str->chars, original->chars);
            copy.data.object = (Object *)str;
            break;
        }

        case OBJ_LIST:
        {
            // Deep copy list
            PiList *original = (PiList *)obj;
            PiList *list = malloc(sizeof(PiList));

            list->object.type = OBJ_LIST;
            list->items = list_create(sizeof(Value)); // PiList contains Value pointers
            list->current = 0;

            for (size_t i = 0; i < LIST_SIZE(original->items); i++)
            {
                // original item
                Value o_item = *(Value *)list_getAt(original->items, i);
                // copied item
                Value c_item = copy_value(o_item);

                list_add(list->items, &c_item);
            }

            copy.data.object = (Object *)list;
            break;
        }

        case OBJ_MAP:
            // PiMap copying not implemented (matches original behavior)
            break;

        default:
            fprintf(stderr, "Unsupported object type for copy\n");
            exit(EXIT_FAILURE);
        }
        break;
    }

    default:
        fprintf(stderr, "Cannot copy value of type %s\n", type_name(val));
        exit(EXIT_FAILURE);
    }

    return copy;
}
void print_value(Value val, bool is_root)
{
    switch (val.type)
    {
    case VAL_NUM:
        // Check if the number is an integer
        if (val.data.number == (long long)val.data.number)
            printf("%lld", (long long)val.data.number);
        else
            printf("%.8f", val.data.number);
        break;
    case VAL_BOOL:
        printf("%s", val.data.boolean ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_OBJ:
        switch (AS_OBJ(val)->type)
        {
        case OBJ_STRING:
            printf("\'%s\'", AS_STRING(val)->chars);
            break;
        case OBJ_LIST:
        {
            int print_limit = 10000; // Set a reasonable limit
            list_t *items = AS_LIST(val)->items;
            int size = items->size;
            printf("[");
            for (int i = 0; i < size; i++)
            {
                print_value(*(Value *)list_getAt(items, i), false);
                if (i < size - 1)
                    printf(", ");
                if (i >= print_limit)
                {
                    printf("... and %d more", size - print_limit);
                    break;
                }
            }
            printf("]");
            break;
        }
        case OBJ_RANGE:
        {
            PiRange *r = AS_RANGE(val);
            printf("[%f..%f:%f]", r->start, r->end, r->step);
            break;
        }
        case OBJ_FUN:
        {
            Function *fn = AS_FUN(val);
            printf("<%s: %p>", fn->name, (void *)fn);
            break;
        }
        case OBJ_MAP:
        case OBJ_CODE:
            break;
        }
        break;
    default:
        printf("Unknown value type: %s", type_name(val));
    }
    if (is_root)
        printf("\n");
    else
        printf(" ");
}

char *type_name(Value val)
{
    switch (val.type)
    {
    case VAL_NUM:
        return "number";
    case VAL_BOOL:
        return "boolean";
    case VAL_NIL:
        return "nil";
    case VAL_OBJ:
        switch (AS_OBJ(val)->type)
        {
        case OBJ_STRING:
            return "string";
        case OBJ_LIST:
            return "list";
        case OBJ_MAP:
            return "map";
        case OBJ_RANGE:
            return "range";
        case OBJ_FUN:
            return "function";
        case OBJ_CODE:
            return "code";
        }
    }

    return NULL;
}
