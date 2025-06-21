#include <stdlib.h>
#include <time.h>

#include "pi_col.h"
#include "../list.h"

static int _compare(const void *a, const void *b)
{
    const Value *va = (const Value *)a;
    const Value *vb = (const Value *)b;

    if (IS_NUM(*va) && IS_NUM(*vb))
    {
        double diff = AS_NUM(*va) - AS_NUM(*vb);
        return (diff < 0) ? -1 : (diff > 0);
    }
    else if (IS_STRING(*va) && IS_STRING(*vb))
        return strcmp(AS_CSTRING(*va), AS_CSTRING(*vb));

    // Should not reach here due to earlier type check
    return 0;
}

/**
 * @brief Removes the last element from a list or character from a string and returns it.
 *
 * This function takes a list or string as input and removes the last element/character.
 * If the input is a list, the last element is removed and returned.
 * If the input is a string, the last character is removed and returned as a new string.
 * If the input is neither, an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return The last element or character from the list or string.
 */
Value pi_pop(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[pop] expects at least one argument.");

    Value arg = argv[0];

    if (IS_LIST(arg))
    {
        list_t *list = AS_CLIST(arg);
        if (list->size == 0)
            error("[pop] Cannot pop from an empty list.");
        return *(Value *)list_pop(list);
    }
    else if (IS_STRING(arg))
    {

        PiString *str = ((PiString *)AS_OBJ(arg));
        int len = str->length;
        if (len == 0)
            error("[pop] Cannot pop from an empty string.");

        // Return the last character as a new string
        char ch[2] = {str->chars[len - 1], '\0'};

        // Resize the original string in place (if desired), or just return the popped character
        str->length -= 1;
        str->chars[len - 1] = '\0';

        return NEW_OBJ(new_pistring(strdup(ch)));
    }
    else
        error("[pop] Argument must be a list or a string.");

    return NEW_NIL();
}

/**
 * @brief Adds elements to the end of a list or characters to the end of a string.
 *
 * This function takes a list or string as the first argument and appends additional
 * elements/characters to it. If the first argument is a list, all subsequent arguments
 * are appended as elements. If it's a string, each argument must be a string of length 1,
 * which will be appended as characters. If the first argument is neither, an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return The new length of the list or string after pushing.
 */
Value pi_push(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[push] expects at least two arguments.");

    Value target = argv[0];

    if (IS_LIST(target))
    {
        list_t *list = AS_CLIST(target);
        for (int i = 1; i < argc; i++)
            list_add(list, &argv[i]);

        return NEW_NUM(list->size);
    }
    else if (IS_STRING(target))
    {
        PiString *str = (PiString *)AS_OBJ(target);

        for (int i = 1; i < argc; i++)
        {
            if (!IS_STRING(argv[i]))
                error("[push] When pushing to a string, all values must be strings.");

            PiString *_arg = (PiString *)AS_OBJ(argv[i]);
            if (_arg->length != 1)
                error("[push] Only single-character strings can be pushed to a string.");

            // Append the character
            char ch = _arg->chars[0];
            str->chars = realloc(str->chars, str->length + 2); // +1 for new char, +1 for '\0'
            str->chars[str->length] = ch;
            str->length += 1;
            str->chars[str->length] = '\0';
        }

        return NEW_NUM(str->length);
    }
    else
        error("[push] First argument must be a list or a string.");

    return NEW_NIL();
}

/**
 * @brief Retrieves the last element from a list or character from a string without removing it.
 *
 * This function takes a list or string as input and returns the last element/character
 * without modifying the input. If the input is a list, the last element is returned.
 * If the input is a string, the last character is returned as a one-character string.
 * If the input is neither, or is empty, an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return The last element or character.
 */
Value pi_peek(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[peek] expects at least one argument.");

    Value arg = argv[0];

    if (IS_LIST(arg))
    {
        list_t *list = AS_CLIST(arg);
        if (list->size == 0)
            error("[peek] Cannot peek from an empty list.");
        return *(Value *)list_getAt(list, list->size - 1);
    }
    else if (IS_STRING(arg))
    {
        PiString *str = (PiString *)AS_OBJ(arg);
        int len = str->length;
        if (len == 0)
            error("[peek] Cannot peek from an empty string.");

        // Return the last character as a one-character string
        char ch[2] = {str->chars[len - 1], '\0'};
        return NEW_OBJ(new_pistring(strdup(ch)));
    }
    else
        error("[peek] Argument must be a list or a string.");

    return NEW_NIL();
}

/**
 * @brief Checks if a list, string, or map is empty.
 *
 * This function takes one argument and returns true if the list, string, or map is empty,
 * false otherwise. If the input is not a list, string, or map, an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return true if the input is empty, false otherwise.
 */
Value pi_empty(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[empty] expects at least one argument.");

    Value arg = argv[0];

    if (IS_LIST(arg))
    {
        list_t *list = AS_CLIST(arg);
        return NEW_BOOL(list->size == 0);
    }
    else if (IS_STRING(arg))
    {
        PiString *str = (PiString *)AS_OBJ(arg);
        return NEW_BOOL(str->length == 0);
    }
    else if (IS_MAP(arg))
    {
        PiMap *map = AS_MAP(arg);
        return NEW_BOOL(map->table->size == 0);
    }
    else
        error("[empty] Argument must be a list, string, or map.");

    return NEW_NIL();
}

/**
 * @brief Sorts a list in-place in ascending order.
 *
 * This function takes one argument: a list. It sorts the list in-place using the default
 * comparison for supported types (numbers and strings). All elements must be of the same
 * type and either all numbers or all strings. Mixed types or unsupported types will raise an error.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return nil
 */
Value pi_sort(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[sort] expects one argument.");

    Value arg = argv[0];

    if (!IS_LIST(arg))
        error("[sort] Argument must be a list.");

    list_t *list = AS_CLIST(arg);

    if (list->size <= 1)
        return NEW_NIL(); // Nothing to sort

    Value first = (*(Value *)list_getAt(list, 0));

    if (!IS_STRING(first) && !IS_NUM(first))
        error("[sort] List elements must all be numbers or strings.");

    for (int i = 1; i < list->size; i++)
    {
        Value item = (*(Value *)list_getAt(list, i));
        if (item.type != first.type)
            error("[sort] List elements must all be of the same type.");
    }

    // Comparator for qsort

    qsort(list->data, list->size, sizeof(Value), _compare);

    return NEW_NIL();
}

/**
 * @brief Inserts a value into a list or string at a specified index.
 *
 * For lists, the value is inserted directly.
 * For strings, only single-character strings can be inserted.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments passed.
 * @param argv Arguments (collection, index, value).
 * @return The modified collection (same reference).
 */
Value pi_insert(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
        error("[insert] expects 3 arguments at least: collection, index, value.");

    Value collection = argv[0];
    Value _index = argv[1];
    Value value = argv[2];

    int index = as_number(_index);

    if (IS_LIST(collection))
    {
        list_t *list = AS_CLIST(collection);
        if (index < 0 || index > list->size)
            error("[insert] Index out of bounds for list.");

        list_addAt(list, index, &value);
        return collection;
    }
    else if (IS_STRING(collection))
    {
        PiString *str = AS_STRING(collection);

        char *_str = as_string(value);

        index = get_index(index, str->length);

        // Allocate new space for +1 character
        int new_len = str->length + strlen(_str);
        char *new_chars = malloc(new_len + 1); // +1 for null terminator

        // Copy before index
        memcpy(new_chars, str->chars, index);
        // Insert new char
        for (int i = 0; i < strlen(_str); i++)
            new_chars[index + i] = _str[i];

        // Copy after index
        memcpy(new_chars + index + strlen(_str), str->chars + index, str->length - index);
        new_chars[new_len] = '\0';

        // Replace original string content
        free(str->chars);
        str->chars = new_chars;
        str->length = new_len;

        return collection;
    }

    error("[insert] First argument must be a list or string.");
    return NEW_NIL(); // unreachable
}

/**
 * @brief Removes an element from a list or a character from a string at the given index.
 *
 * For lists: returns the removed element.
 * For strings: returns the removed character as a new string.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments passed (must be 2).
 * @param argv Arguments: collection, index.
 * @return The removed element or character.
 */
Value pi_remove(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[remove] expects two arguments at least: collection and index.");

    Value collection = argv[0];
    Value _index = argv[1];

    int index = as_number(_index);

    // Handle list removal
    if (IS_LIST(collection))
    {
        list_t *list = AS_CLIST(collection);
        return *(Value *)list_remove(list, index); // Assumes list_removeAt returns a pointer to Value
    }

    // Handle string character removal
    else if (IS_STRING(collection))
    {
        PiString *str = AS_STRING(collection);

        index = get_index(index, str->length);

        // Get the character being removed
        char removed = str->chars[index];

        // Create a new string with the character
        char ch[2] = {removed, '\0'};
        Value removed_val = NEW_OBJ(new_pistring(strdup(ch)));

        // Shift string content left to remove character
        memmove(&str->chars[index], &str->chars[index + 1], str->length - index);
        str->length--;
        str->chars[str->length] = '\0'; // Null-terminate

        return removed_val;
    }

    error("[remove] First argument must be a list or string.");

    return NEW_NIL();
}

/**
 * @brief Prepends one or more values to the beginning of a collection.
 *
 * Supports both lists and strings. For lists, any type of value is allowed.
 * For strings, all values must be strings or characters.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: collection followed by values to prepend.
 * @return The new size of the collection.
 */
Value pi_unshift(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[unshift] expects at least two arguments: collection and values.");

    Value target = argv[0];

    if (IS_LIST(target))
    {
        list_t *list = AS_CLIST(target);

        // Shift items right and insert in reverse order to maintain input order
        for (int i = 1; i < argc; i++)
            list_addFirst(list, &argv[i]); // Prepend each item at index 0

        return NEW_NUM(list->size);
    }
    else if (IS_STRING(target))
    {
        PiString *str = AS_STRING(target);

        // Calculate total new length
        int total_len = str->length;
        for (int i = argc - 1; i >= 1; i--)
        {
            if (!IS_STRING(argv[i]))
                error("[unshift] All values must be strings when prepending to a string.");
            total_len += AS_STRING(argv[i])->length;
        }

        // Allocate new string
        char *new_chars = malloc(total_len + 1);
        int offset = 0;

        // Copy new items first
        for (int i = argc - 1; i >= 1; i--)
        {
            PiString *s = AS_STRING(argv[i]);
            memcpy(new_chars + offset, s->chars, s->length);
            offset += s->length;
        }

        // Copy old string content
        memcpy(new_chars + offset, str->chars, str->length);
        new_chars[total_len] = '\0';

        // Replace original string content
        free(str->chars);
        str->chars = new_chars;
        str->length = total_len;

        return NEW_NUM(str->length);
    }
    else
        error("[unshift] First argument must be a list or a string.");

    return NEW_NIL(); // Unreachable
}

/**
 * @brief Appends one or more values to the end of a collection.
 *
 * Supports both lists and strings. For lists, any type of value is allowed.
 * For strings, all values must be strings or characters.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: collection followed by values to append.
 * @return The new size of the collection.
 */
Value pi_append(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[append] expects at least two arguments: collection and values.");

    Value target = argv[0];

    if (IS_LIST(target))
    {
        list_t *list = AS_CLIST(target);

        for (int i = 1; i < argc; i++)
            list_add(list, &argv[i]); // Append each value to the end

        return NEW_NUM(list->size);
    }
    else if (IS_STRING(target))
    {
        PiString *str = AS_STRING(target);

        // Calculate new total length
        int total_len = str->length;
        for (int i = 1; i < argc; i++)
        {
            if (!IS_STRING(argv[i]))
                error("[append] All values must be strings when appending to a string.");
            total_len += AS_STRING(argv[i])->length;
        }

        // Allocate new buffer
        char *new_chars = malloc(total_len + 1);
        memcpy(new_chars, str->chars, str->length);

        int offset = str->length;
        for (int i = 1; i < argc; i++)
        {
            PiString *s = AS_STRING(argv[i]);
            memcpy(new_chars + offset, s->chars, s->length);
            offset += s->length;
        }

        new_chars[total_len] = '\0';

        // Replace old string
        free(str->chars);
        str->chars = new_chars;
        str->length = total_len;

        return NEW_NUM(str->length);
    }
    else
        error("[append] First argument must be a list or a string.");

    return NEW_NIL(); // Unreachable
}

/**
 * @brief Checks whether a collection contains a given value or key.
 *
 * For lists, checks if the value is present.
 * For strings, checks if the value is a substring.
 * For maps, checks if the value is a key.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: [collection, value]
 * @return A boolean indicating whether the collection contains the value.
 */
Value pi_contains(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[contains] expects two arguments at least: a collection and a value.");

    Value collection = argv[0];
    Value target = argv[1];

    if (IS_LIST(collection))
    {
        PiList *list = AS_LIST(collection);
        for (int i = 0; i < list->items->size; i++)
        {
            Value item = *(Value *)list_getAt(list->items, i);
            if (equals(item, target))
                return NEW_BOOL(true);
        }
    }
    else if (IS_STRING(collection))
    {
        if (!IS_STRING(target))
            error("[contains] When searching a string, the value must also be a string.");

        PiString *str = AS_STRING(collection);
        PiString *substr = AS_STRING(target);

        if (substr->length == 0 || substr->length > str->length)
            return NEW_BOOL(false);

        for (int i = 0; i <= str->length - substr->length; i++)
        {
            if (strncmp(&str->chars[i], substr->chars, substr->length) == 0)
                return NEW_BOOL(true);
        }
    }
    else if (IS_MAP(collection))
    {
        PiMap *map = AS_MAP(collection);
        return NEW_BOOL(map_has(map, target));
    }
    else
        error("[contains] First argument must be a list, string, or map.");

    return NEW_BOOL(false);
}

/**
 * @brief Returns the index of the first occurrence of a value in a collection.
 *
 * Works for both lists and strings. Returns -1 if the value is not found.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: [collection, value]
 * @return The index of the value in the collection, or -1 if not found.
 */
Value pi_indexOf(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        error("[index_of] expects at least two arguments: a collection and a value.");

    Value collection = argv[0];
    Value target = argv[1];

    if (IS_LIST(collection))
    {
        PiList *list = AS_LIST(collection);
        for (int i = 0; i < list->items->size; i++)
        {
            Value item = *(Value *)list_getAt(list->items, i);
            if (equals(item, target))
                return NEW_NUM(i);
        }
    }
    else if (IS_STRING(collection))
    {
        if (!IS_STRING(target))
            error("[index_of] When searching a string, the target must also be a string.");

        PiString *str = AS_STRING(collection);
        PiString *substr = AS_STRING(target);

        if (substr->length == 0 || substr->length > str->length)
            return NEW_NUM(-1);

        for (int i = 0; i <= str->length - substr->length; i++)
            if (strncmp(&str->chars[i], substr->chars, substr->length) == 0)
                return NEW_NUM(i);
    }
    else
        error("[index_of] First argument must be a list or a string.");

    return NEW_NUM(-1); // Not found
}

/**
 * @brief Reverses a collection in-place (for lists) or returns a reversed string.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [collection]
 * @return The reversed collection.
 */
Value pi_reverse(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        error("[reverse] expects one argument at least: a list or a string.");

    Value input = argv[0];

    if (IS_LIST(input))
    {
        list_t *list = AS_CLIST(input);
        int size = list->size;

        list_t *copy = list_copy(list);
        for (int i = 0; i < size / 2; i++)
        {
            Value *a = (Value *)list_getAt(copy, i);
            Value *b = (Value *)list_getAt(copy, size - i - 1);
            Value tmp = *a;
            *a = *b;
            *b = tmp;
        }

        return NEW_OBJ(new_list(copy)); // reversed in-place
    }
    else if (IS_STRING(input))
    {
        PiString *str = AS_STRING(input);
        int len = str->length;
        char *reversed = malloc(len + 1);

        for (int i = 0; i < len; i++)
            reversed[i] = str->chars[len - i - 1];

        reversed[len] = '\0';
        return NEW_OBJ(new_pistring(reversed));
    }
    else
    {
        error("[reverse] argument must be a list or a string.");
    }

    return NEW_NIL();
}

/**
 * @brief Shuffles a list in-place using Fisher–Yates algorithm.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [list]
 * @return The shuffled list.
 */
Value pi_shuffle(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        error("[shuffle] expects one argument at least: a list.");

    if (!IS_LIST(argv[0]))
        error("[shuffle] argument must be a list.");

    PiList *list = AS_LIST(argv[0]);
    int size = list->items->size;

    // Seed RNG once
    static bool seeded = false;
    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    for (int i = size - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Value *a = (Value *)list_getAt(list->items, i);
        Value *b = (Value *)list_getAt(list->items, j);
        Value tmp = *a;
        *a = *b;
        *b = tmp;
    }

    return argv[0]; // shuffled in-place
}

/**
 * @brief Returns a deep copy of a list or a string.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [collection]
 * @return A new copy of the collection.
 */
Value pi_copy(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        error("[copy] expects one argument at least!.");

    Value input = argv[0];

    if (IS_STRING(input))
    {
        PiString *str = AS_STRING(input);
        return NEW_OBJ(new_pistring(strdup(str->chars)));
    }
    else if (IS_LIST(input))
    {
        PiList *orig = AS_LIST(input);
        list_t *copied_items = list_create(sizeof(Value));

        for (int i = 0; i < orig->items->size; i++)
        {
            Value *item = (Value *)list_getAt(orig->items, i);
            list_add(copied_items, item); // shallow copy of elements
        }

        PiList *result = (PiList *)new_list(copied_items);
        result->is_numeric = orig->is_numeric;
        result->is_matrix = orig->is_matrix;

        return NEW_OBJ(result);
    }

    error("[copy] only works with lists or strings.");
    return NEW_NIL();
}

Value pi_slice(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
        error("[slice] expects 3 arguments at least: collection, start, end.");

    Value collection = argv[0];
    Value start = argv[1];
    Value end = argv[2];

    if (!IS_LIST(collection) && !IS_STRING(collection))
        error("[slice] first argument must be a list or a string.");

    if (!IS_NUM(start) || !IS_NUM(end))
        error("[slice] second and third arguments must be numbers.");

    int len = COL_LENGTH(collection);
    int start_index = get_index(as_number(start), len);
    int end_index = get_index(as_number(end), len);

    if (start_index > end_index)
        error("[slice] start index must be less than or equal to end index.");

    if (IS_LIST(collection))
    {
        PiList *list = AS_LIST(collection);

        list_t *sliced_items = list_create(sizeof(Value));
        for (int i = start_index; i <= end_index; i++)
        {
            Value *item = (Value *)list_getAt(list->items, i);
            list_add(sliced_items, item);
        }

        PiList *result = (PiList *)new_list(sliced_items);
        result->is_numeric = list->is_numeric;
        result->is_matrix = list->is_matrix;

        return NEW_OBJ(result);
    }
    else if (IS_STRING(collection))
    {
        PiString *str = AS_STRING(collection);

        char *sliced_chars = malloc(end_index - start_index + 1);
        for (int i = start_index; i <= end_index; i++)
            sliced_chars[i - start_index] = str->chars[i];

        sliced_chars[end_index - start_index + 1] = '\0';

        return NEW_OBJ(new_pistring(sliced_chars));
    }

    error("[slice] only works with lists or strings.");

    return NEW_NIL(); // unreachable
}

Value pi_len(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[len] expects at least one argument.");

    switch (OBJ_TYPE(argv[0]))
    {
    case OBJ_LIST:
        return NEW_NUM(AS_CLIST(argv[0])->size);
    case OBJ_STRING:
        return NEW_NUM(AS_STRING(argv[0])->length);
    case OBJ_MAP:
        return NEW_NUM(AS_CMAP(argv[0])->size);
    default:
        return NEW_NIL();
    }
}


Value pi_range(vm_t *vm, int argc, Value *argv)
{
    double start = 0;
    double end = 0;
    double step = 1;

    if (argc == 1)
    {
        // range(end)
        if (!IS_NUM(argv[0]))
            error("[range] Expected a number as the end value.");
        end = AS_NUM(argv[0]);
    }
    else if (argc == 2)
    {
        // range(start, end)
        if (!IS_NUM(argv[0]) || !IS_NUM(argv[1]))
            error("[range] Expected numbers for start and end values.");

        start = AS_NUM(argv[0]);
        end = AS_NUM(argv[1]);
    }
    else if (argc == 3)
    {
        // range(start, end, step)
        if (!IS_NUM(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]))
            error("[range] Expected numbers for start, end, and step values.");

        start = AS_NUM(argv[0]);
        end = AS_NUM(argv[1]);
        step = AS_NUM(argv[2]);
        if (step == 0)
            error("[range] Step cannot be zero.");
    }
    else
        error("[range] Expected 1 to 3 arguments.");

    Object *range_obj = new_range(start, end, step);
    return NEW_OBJ(range_obj);
}
