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
            return ERROR_COMPARE;
        }
        return ERROR_COMPARE;
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
            return ERROR_COMPARE;

    default:
        return ERROR_COMPARE;
    }
}

/**
 * @brief Escapes special characters in a string.
 *
 * Takes a string with special characters escaped using C-style escape
 * sequences and returns a new string with the escape sequences converted
 * into their corresponding special characters.
 *
 * @param src The string to unescape.
 * @return A new string with the escape sequences converted.
 */
static char *unescape_string(const char *src)
{
    size_t len = strlen(src);
    char *dest = malloc(len + 1); // worst case: same length
    char *out = dest;

    for (const char *p = src; *p; ++p)
    {
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
            case 'n':
                // Newline character
                *out++ = '\n';
                break;
            case 't':
                // Tab character
                *out++ = '\t';
                break;
            case '\\':
                // Backslash character
                *out++ = '\\';
                break;
            case '"':
                // Double quote character
                *out++ = '"';
                break;
            case 'r':
                // Carriage return character
                *out++ = '\r';
                break;
            default:
                // Unknown escape sequence, treat as raw character
                *out++ = *p;
                break;
            }
        }
        else
            *out++ = *p;
    }

    *out = '\0';
    return dest;
}

/**
 * @brief Creates a new Value from a given token.
 *
 * This function converts a token into a Pi Value based on its type.
 * It supports various token types such as numbers, strings,
 * identifiers, booleans, and nil.
 *
 * @param token The token to convert.
 * @return A Value representing the token.
 */
Value new_value(token_t token)
{
    Value val; // The value to be returned

    switch (token.type)
    {
    case TK_NUM:
        // Convert numeric token to a number value
        val.type = VAL_NUM;
        val.data.number = tk_double(token);
        break;

    case TK_STR:
    {
        // Convert string token to a string object
        const char *raw = tk_string(token);
        char *unescaped = unescape_string(raw); // Function to unescape special characters
        val = NEW_OBJ(new_pistring(strdup(unescaped)));
        free(unescaped); // Free the temporary unescaped string
        break;
    }

    case TK_ID:
        // Convert identifier token to a string object
        val = NEW_OBJ(new_pistring(tk_string(token)));
        break;

    case TK_TRUE:
    case TK_FALSE:
        // Convert boolean token to a boolean value
        val.type = VAL_BOOL;
        val.data.boolean = tk_bool(token);
        break;

    case TK_NIL:
        // Convert nil token to a nil value
        val.type = VAL_NIL;
        break;

    default:
        // Handle unexpected token types
        error("Unexpected token value: %s", tk_string(token));
    }

    return val;
}

/**
 * @brief Converts a value to a number
 *
 * This function attempts to convert a given Pi value to a number.
 * Conversion is based on the type of the value.
 *
 * @param val The value to convert.
 * @return A number representation of the value.
 */
double as_number(Value val)
{
    switch (val.type)
    {
    case VAL_NUM:
        // Numbers are already numbers
        return val.data.number;
    case VAL_BOOL:
        // Boolean values can be converted to 0 or 1
        return val.data.boolean ? 1.0 : 0.0;
    case VAL_NIL:
        // Nil values are equivalent to 0
        return 0.0;
    case VAL_OBJ:
        if (AS_OBJ(val)->type == OBJ_STRING)
        {
            // Attempt to parse the string as a number
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

/**
 * @brief Converts a value to a boolean
 *
 * This function attempts to convert a given Pi value to a boolean.
 * Conversion is based on the type of the value.
 *
 * @param val The value to convert.
 * @return A boolean representation of the value.
 */
bool as_bool(Value val)
{
    switch (val.type)
    {
    case VAL_BOOL:
        // Directly return the boolean value
        return val.data.boolean;
    case VAL_NUM:
        // Numbers are true if non-zero
        return val.data.number != 0.0;
    case VAL_NIL:
        // Nil values are false
        return false;
    case VAL_OBJ:
        switch (AS_OBJ(val)->type)
        {
        case OBJ_STRING:
            // Strings are true if non-empty
            return AS_STRING(val)->length > 0;
        case OBJ_LIST:
            // Lists are true if they have items
            return LIST_SIZE(AS_LIST(val)->items) > 0;
        case OBJ_MAP:
            // Maps are true if they have key-value pairs
            return ht_length(AS_MAP(val)->table) > 0;
        case OBJ_RANGE:
            // Ranges are true if start and end are different
            return AS_RANGE(val)->start != AS_RANGE(val)->end;
        default:
            // Other object types default to true
            return true;
        }
    default:
        // Error if value cannot be converted
        error("Expected a boolean, but got %s", type_name(val));
    }
}

/**
 * @brief Converts a value to a string
 *
 * The function converts a given Pi value to a string.
 * For numerical values, it converts them to a string using the `%g` format specifier.
 * For boolean values, it returns the string "true" or "false".
 * For nil values, it returns the string "nil".
 * For list and map values, it recursively converts the elements to strings and concatenates them.
 * For functions, it returns a string in the format `<function name: pointer>`.
 * For range values, it returns a string in the format `<start>..=<end>`.
 * For code values, it returns an empty string.
 *
 * @param val The Pi value to be converted
 * @return A string representation of the value
 */
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
        return val.data.boolean ? strdup("true") : strdup("false");
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

            // case OBJ_MAP:
            // {
            //     PiMap *map = AS_MAP(val);
            //     list_t *keys = map->table->_keys;
            //     int size = list_size(keys);
            //     if (size == 0)
            //         return strdup("{}");

            //     size_t buffer_size = 2; // Start with "{}"
            //     char *result = strdup("{");

            //     for (int i = 0; i < size; i++)
            //     {
            //         if (i > 0)
            //         {
            //             buffer_size += 2;
            //             result = realloc(result, buffer_size);
            //             strcat(result, ", ");
            //         }

            //         char *key = string_get(keys, i);
            //         char *value = as_string(*(Value *)ht_get(map->table, key));

            //         buffer_size += strlen(key) + strlen(": ") + strlen(value);
            //         result = realloc(result, buffer_size);
            //         strcat(result, key);
            //         strcat(result, ": ");
            //         strcat(result, value);

            //         free(value);
            //     }

            //     buffer_size++;
            //     result = realloc(result, buffer_size);
            //     strcat(result, "}");

            //     return result;
            // }

        case OBJ_MAP:
        {
            PiMap *map = AS_MAP(val);
            char **keys = map->table->_keys;
            int size = ht_length(map->table);

            if (size == 0)
                return strdup("{}");

            size_t buffer_size = 2; // Start with "{}"
            char *result = strdup("{");

            for (int i = 0; i < size; i++)
            {
                char *key = keys[i];
                char *value = as_string(*(Value *)ht_get(map->table, key));

                // Add comma and space if not the first entry
                if (i > 0)
                {
                    buffer_size += 2;
                    result = realloc(result, buffer_size);
                    strcat(result, ", ");
                }

                buffer_size += strlen(key) + 2 + strlen(value) + 1; // key + ": " + value + null
                result = realloc(result, buffer_size);
                strcat(result, key);
                strcat(result, ": ");
                strcat(result, value);

                free(value);
            }

            buffer_size += 2;
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
        case OBJ_SPRITE:
        {
            ObjSprite *sprite = AS_SPRITE(val);
            char *result = (char *)malloc(64);
            sprintf(result, "<sprite %dx%d>", sprite->width, sprite->height);
            return result;
        }
        case OBJ_RANGE:
        case OBJ_CODE:
            break;
        }
    }
    default:
        return NULL;
    }

    return NULL;
}

/**
 * @brief Converts a Value to a list_t pointer if the Value is a list.
 * @param val The Value to convert
 * @return A pointer to the list_t structure if the Value is a list, else NULL
 */
list_t *as_list(Value val)
{
    if (val.type == VAL_OBJ && OBJ_TYPE(val) == OBJ_LIST)
        return AS_LIST(val)->items;

    error("Expected a list, but got %s", type_name(val));
}
/**
 * @brief Checks if a Value is numeric.
 *
 * This function determines if the given Value represents a numeric type.
 * It considers numbers, booleans, and nil values as numeric. For string
 * objects, it attempts to parse the string as a number.
 *
 * @param val The Value to check for numeric type.
 * @return True if the Value is numeric, otherwise false.
 */
bool is_numeric(Value val)
{
    // Directly numeric types
    if (val.type == VAL_NUM || val.type == VAL_BOOL || val.type == VAL_NIL)
        return true;

    // Check if the Value is a string object
    if (val.type == VAL_OBJ && OBJ_TYPE(val) == OBJ_STRING)
    {
        char *str_value = AS_STRING(val)->chars;
        char *end_ptr;
        // Attempt to convert the string to a double
        strtod(str_value, &end_ptr);

        // Check if the conversion consumed the entire string
        return *end_ptr == '\0';
    }

    // Non-numeric for all other types
    return false;
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
            error("Unsupported object type for copy");
        }
        break;
    }

    default:
        error("Unsupported object type for copy");
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
        case OBJ_SPRITE:
        {
            ObjSprite *sprite = AS_SPRITE(val);
            printf("<sprite %dx%d>", sprite->width, sprite->height);
            break;
        }
        case OBJ_MAP:
        case OBJ_CODE:
            break;
        }
        break;
    default:
        error("Unknown value type: %s", type_name(val));
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
        case OBJ_FILE:
            return "file";
        case OBJ_MODEL3D:
            return "model3d";
        case OBJ_IMAGE:
            return "image";
        case OBJ_SPRITE:
            return "sprite";
        default:
            return "undefined";
        }
    }

    return NULL;
}
