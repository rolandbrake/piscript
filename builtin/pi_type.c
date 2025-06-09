#include "pi_type.h"

// Returns true if the argument is a list
Value pi_isList(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[is_list] expects one argument.");

    return NEW_BOOL(IS_LIST(argv[0]));
}

// Returns true if the argument is a map
Value pi_isMap(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[is_map] expects one argument.");

    return NEW_BOOL(IS_MAP(argv[0]));
}

// Returns true if the argument is numeric (integer or float)
Value pi_isNum(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[is_num] expects one argument.");

    return NEW_BOOL(is_numeric(argv[0]));
}

// Returns true if the argument is a string
Value pi_isStr(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[is_str] expects one argument.");

    return NEW_BOOL(IS_STRING(argv[0]));
}

// Returns true if the argument is a boolean
Value pi_isBool(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[is_bool] expects one argument.");

    return NEW_BOOL(IS_BOOL(argv[0]));
}

// Converts the argument to a list if possible, else throws an error

// Converts the argument to a number if possible, else throws an error
Value pi_asNum(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[as_num] expects one argument.");

    if (is_numeric(argv[0]))
        return NEW_NUM(as_number(argv[0]));
    else
        error("[as_num] argument is not numeric.");

    return NEW_NIL();
}

// Converts the argument to a string if possible, else throws an error
Value pi_asStr(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[as_str] expects one argument.");

    return NEW_OBJ(new_pistring(as_string(argv[0])));

    return NEW_NIL();
}

// Converts the argument to a boolean if possible, else throws an error
Value pi_asBool(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[as_bool] expects one argument.");

    return NEW_BOOL(as_bool(argv[0]));

    return NEW_NIL();
}
