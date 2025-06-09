#include "pi_fun.h"
#include "../pi_func.h"
#include "../list.h"

/**
 * @brief Maps a function to every item in a list.
 *
 * This function takes two arguments: a function and a list. It applies the
 * function to each item in the list and returns a new list with the results.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return A new list with the results of the function applied to each item.
 */
Value _pi_map(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_FUN(argv[1]))
        error("map(fn, list): expects a function and a list");

    PiList *input = AS_LIST(argv[0]);
    Function *fn = AS_FUN(argv[1]);
    list_t *list = list_create(sizeof(Value));

    int size = input->items->size;
    for (int i = 0; i < size; i++)
    {
        Value *item = (Value *)list_getAt(input->items, i);
        Value ret_val = call_func(vm, fn, 1, item);
        list_add(list, &ret_val);
    }

    PiList *result = (PiList *)new_list(list);
    result->is_numeric = false;
    result->is_matrix = false;

    // Update numeric flag if all values are numbers
    result->is_numeric = true;

    size = result->items->size;
    for (int i = 0; i < size; i++)
    {
        Value item = *(Value *)list_getAt(result->items, i);
        if (!IS_NUM(item))
        {
            result->is_numeric = false;
            break;
        }
    }

    return NEW_OBJ(result);
}

/**
 * @brief Returns a new list containing the items for which the callback function returns true.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 2).
 * @param argv Arguments: [callback, list]
 * @return A new list with the filtered items.
 */
Value pi_filter(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_FUN(argv[1]))
        error("filter(fn, list): expects a function and a list");

    PiList *input = AS_LIST(argv[0]);
    Function *fn = AS_FUN(argv[1]);
    list_t *list = list_create(sizeof(Value));

    int size = input->items->size;
    for (int i = 0; i < size; i++)
    {
        Value *item = (Value *)list_getAt(input->items, i);
        Value ret_val = call_func(vm, fn, 1, item);
        if (as_bool(ret_val))
            list_add(list, item);
    }

    PiList *result = (PiList *)new_list(list);
    result->is_numeric = input->is_numeric;
    result->is_matrix = false; // filtering may disrupt matrix structure

    return NEW_OBJ(result);
}

/**
 * @brief Applies a function against an accumulator and each value of the list (from left to right) to reduce it to a single value.
 *
 * This function takes a list, a function, and an optional initial value. It applies the function to an accumulator and
 * each element of the list in turn, reducing the list to a single accumulated value. If an initial value is provided,
 * it is used as the starting value of the accumulator. Otherwise, the first element of the list is used.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 2 or 3).
 * @param argv Arguments: [callback, list, optional initial value]
 * @return The accumulated value after applying the function across all elements.
 */

Value pi_reduce(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2 || !IS_LIST(argv[0]) || !IS_FUN(argv[1]))
        error("reduce(fn, list, [initial]): expects a function, a list, and optional initial value");

    PiList *input = AS_LIST(argv[0]);
    Function *fn = AS_FUN(argv[1]);    
    Value acc = (argc == 3) ? argv[2] : *(Value *)list_getAt(input->items, 0);
    int start = (argc == 3) ? 0 : 1;

    int size = input->items->size;
    for (int i = start; i < size; i++)
    {
        Value item = *(Value *)list_getAt(input->items, i);
        acc = call_funcv(vm, fn, 2, acc, item);
    }

    return acc;
}

/**
 * @brief Finds the index of the first element in a collection that satisfies a callback.
 *
 * The function takes a collection (list or string) and a callback function. It returns
 * the index of the first item for which the callback returns true. If none match, -1 is returned.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: [callback, collection]
 * @return Index of the first matching item, or -1 if none match.
 */
Value pi_find(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_FUN(argv[1]))
        error("[find] expects two arguments: a function and a collection.");

    Value collection = argv[0];
    Function *fn = AS_FUN(argv[1]);

    if (IS_LIST(collection))
    {
        PiList *list = AS_LIST(collection);
        for (int i = 0; i < list->items->size; i++)
        {
            Value *item = (Value *)list_getAt(list->items, i);
            Value result = call_func(vm, fn, 1, item);
            if (as_bool(result))
                return NEW_NUM(i);
        }
    }
    else if (IS_STRING(collection))
    {
        PiString *str = AS_STRING(collection);
        char ch[2] = {'\0', '\0'};
        for (int i = 0; i < str->length; i++)
        {
            ch[0] = str->chars[i];
            Value arg = NEW_OBJ(new_pistring(strdup(ch)));
            Value result = call_func(vm, fn, 1, &arg);
            if (as_bool(result))
                return NEW_NUM(i);
        }
    }
    else
        error("[find] Second argument must be a list or a string.");

    return NEW_NUM(-1); // Not found
}
