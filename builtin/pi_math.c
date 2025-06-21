#include <math.h>
#include <time.h>
#include <stdlib.h> // For qsort

#include "pi_math.h"
#include "../common.h"

static uint32_t state = 2463534242; // Initial seed

/**
 * @brief Return the floor of a number or each element in a list.
 *
 * @param vm The virtual machine.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return The floor of a number or a new list of floors.
 */
Value pi_floor(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[floor] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(floor(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[floor] All elements in the list must be numeric.");

            double _floor = floor(as_number(item));
            Value val = NEW_NUM(_floor);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[floor] expects a numeric value or a list of numberic values.");
    return NEW_NIL();
}

/**
 * @brief Returns the ceiling of a number or each number in a list.
 *
 * This function accepts either a single numeric value or a list of numeric values.
 * If a single number is passed, it returns its ceiling.
 * If a list is passed, it returns a new list with the ceiling of each number.
 */
Value pi_ceil(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[ceil] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(ceil(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[ceil] All elements in the list must be numeric.");

            double _ceil = ceil(as_number(item));
            Value val = NEW_NUM(_ceil);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[ceil] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Returns the rounded value of a number or each number in a list.
 *
 * This function accepts either a single numeric value or a list of numeric values.
 * If a single number is passed, it returns its rounded value.
 * If a list is passed, it returns a new list with each number rounded.
 */
Value pi_round(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[round] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(round(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[round] All elements in the list must be numeric.");

            double _round = round(as_number(item));
            Value val = NEW_NUM(_round);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[round] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Returns the square root of a number or each number in a list.
 *
 * Accepts either a single numeric value or a list of numeric values.
 * If a number is passed, returns its square root.
 * If a list is passed, returns a new list with the square root of each element.
 */
Value pi_sqrt(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[sqrt] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(sqrt(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[sqrt] All elements in the list must be numeric.");

            double _sqrt = sqrt(as_number(item));
            Value val = NEW_NUM(_sqrt);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[sqrt] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Returns the sine of a number or each number in a list (in radians).
 *
 * Accepts either a single numeric value or a list of numeric values.
 * If a number is passed, returns its sine.
 * If a list is passed, returns a new list with the sine of each element.
 */
Value pi_sin(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[sin] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(sin(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[sin] All elements in the list must be numeric.");

            double _sin = sin(as_number(item));
            Value val = NEW_NUM(_sin);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[sin] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Returns the cosine of a number or each number in a list (in radians).
 *
 * Accepts either a single numeric value or a list of numeric values.
 * If a number is passed, returns its cosine.
 * If a list is passed, returns a new list with the cosine of each element.
 */
Value pi_cos(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[cos] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(cos(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[cos] All elements in the list must be numeric.");

            double _cos = cos(as_number(item));
            Value val = NEW_NUM(_cos);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[cos] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Computes the arcsine (inverse sine) of a number or a list of numbers.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a single numeric value or a list of numeric values).
 * @return The arcsine of the number or a list of arcsines.
 */
Value pi_asin(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[asin] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double val = as_number(arg);
        if (val < -1.0 || val > 1.0)
            error("[asin] argument must be in the range [-1, 1].");

        double result = asin(val);
        return NEW_NUM(result);
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[asin] All elements in the list must be numeric.");

            double val = as_number(item);
            if (val < -1.0 || val > 1.0)
                error("[asin] All list elements must be in the range [-1, 1].");

            double res = asin(val);
            Value val_obj = NEW_NUM(res);
            list_add(result, &val_obj);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[asin] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Returns the tangent of a number or each number in a list (in radians).
 *
 * Accepts either a single numeric value or a list of numeric values.
 * If a number is passed, returns its tangent.
 * If a list is passed, returns a new list with the tangent of each element.
 */
Value pi_tan(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[tan] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(tan(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[tan] All elements in the list must be numeric.");

            double _tan = tan(as_number(item));
            Value val = NEW_NUM(_tan);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[tan] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Computes the arccosine (inverse cosine) of a number or a list of numbers.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a single numeric value or a list of numeric values).
 * @return The arccosine of the number or a list of arccosines.
 */
Value pi_acos(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[acos] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double val = as_number(arg);
        if (val < -1.0 || val > 1.0)
            error("[acos] argument must be in the range [-1, 1].");

        double result = acos(val);
        return NEW_NUM(result);
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[acos] All elements in the list must be numeric.");

            double val = as_number(item);
            if (val < -1.0 || val > 1.0)
                error("[acos] All list elements must be in the range [-1, 1].");

            double res = acos(val);
            Value val_obj = NEW_NUM(res);
            list_add(result, &val_obj);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[acos] expects a numeric value or a list of numeric values.");

    return NEW_NIL(); // Unreachable
}

/**
 * @brief Computes the arctangent (inverse tangent) of a number or a list of numbers.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a single numeric value or a list of numeric values).
 * @return The arctangent of the number or a list of arctangents.
 */
Value pi_atan(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[atan] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double val = as_number(arg);
        double result = atan(val);
        return NEW_NUM(result);
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[atan] All elements in the list must be numeric.");

            double val = as_number(item);
            double res = atan(val);
            Value val_obj = NEW_NUM(res);
            list_add(result, &val_obj);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[atan] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Converts radians to degrees for a number or a list of numbers.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a single numeric value or a list of numeric values).
 * @return The degrees equivalent of the input radians or a list of such values.
 */
Value pi_deg(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[deg] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double val = as_number(arg);
        double result = val * RAD_TO_DEG;
        return NEW_NUM(result);
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[deg] All elements in the list must be numeric.");

            double val = as_number(item);
            double res = val * RAD_TO_DEG;
            Value val_obj = NEW_NUM(res);
            list_add(result, &val_obj);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[deg] expects a numeric value or a list of numeric values.");

    return NEW_NIL();
}

/**
 * @brief Converts degrees to radians for a number or a list of numbers.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a single numeric value or a list of numeric values).
 * @return The radians equivalent of the input degrees or a list of such values.
 */
Value pi_rad(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[rad] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];    

    if (is_numeric(arg))
    {
        double val = as_number(arg);
        double result = val * DEG_TO_RAD;
        return NEW_NUM(result);
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[rad] All elements in the list must be numeric.");

            double val = as_number(item);
            double res = val * DEG_TO_RAD;
            Value val_obj = NEW_NUM(res);
            list_add(result, &val_obj);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[rad] expects a numeric value or a list of numeric values.");

    return NEW_NIL();
}

/**
 * @brief Calculates the sum of all numeric elements in a list.
 *
 * This function takes a list of numeric values and returns their total sum.
 * If any element in the list is not numeric, an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a list of numeric values).
 * @return The sum of all numbers in the list.
 */
Value pi_sum(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[sum] expects a single list of numeric values.");

    list_t *input = AS_CLIST(argv[0]);
    double total = 0.0;

    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
            error("[sum] All elements in the list must be numeric.");
        total += as_number(item);
    }

    return NEW_NUM(total);
}

/**
 * @brief Calculates the exponential (e^x) of a number or each element in a list.
 *
 * This function accepts either a single numeric value or a list of numeric values.
 * It returns e raised to the power of the number(s).
 * If a list is provided, the result is a new list containing the exponential of each element.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a numeric value or a list of numeric values).
 * @return The exponential of the number or a list of exponentials.
 */
Value pi_exp(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[exp] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(exp(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[exp] All elements in the list must be numeric.");

            double val = exp(as_number(item));
            Value v = NEW_NUM(val);
            list_add(result, &v);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[exp] expects a numeric value or a list of numeric values.");

    return NEW_NIL();
}

/**
 * @brief Calculates the base-2 logarithm of a number or each element in a list.
 *
 * This function accepts a single numeric value or a list of numeric values.
 * It returns the base-2 logarithm (log2) of the number(s).
 * If a list is provided, the result is a new list containing the log2 of each element.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a numeric value or a list of numeric values).
 * @return The base-2 logarithm of the number or a list of logarithms.
 */
Value pi_log2(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[log2] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double num = as_number(arg);
        if (num <= 0)
            error("[log2] input must be positive.");
        return NEW_NUM(log2(num));
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[log2] All elements in the list must be numeric.");

            double val = as_number(item);
            if (val <= 0)
                error("[log2] All elements must be positive.");

            Value v = NEW_NUM(log2(val));
            list_add(result, &v);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[log2] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * @brief Calculates the base-10 logarithm of a number or each element in a list.
 *
 * This function accepts a single numeric value or a list of numeric values.
 * It returns the base-10 logarithm (log10) of the number(s).
 * If a list is provided, the result is a new list containing the log10 of each element.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (a numeric value or a list of numeric values).
 * @return The base-10 logarithm of the number or a list of logarithms.
 */
Value pi_log10(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[log10] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
    {
        double num = as_number(arg);
        if (num <= 0)
            error("[log10] input must be positive.");
        return NEW_NUM(log10(num));
    }
    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[log10] All elements in the list must be numeric.");

            double val = as_number(item);
            if (val <= 0)
                error("[log10] All elements must be positive.");

            Value v = NEW_NUM(log10(val));
            list_add(result, &v);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[log10] expects a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * Computes the power of a number or each element in a list raised to the given exponent.
 *
 * @param vm The virtual machine.
 * @param argc Number of arguments (expects exactly 2).
 * @param argv The arguments: base (number or list), exponent (number).
 * @return The result of base^exponent (number or list).
 */
Value pi_pow(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2)
        error("[pow] expects exactly two arguments: base and exponent.");

    Value base = argv[0];
    Value exponent = argv[1];

    if (!is_numeric(exponent))
        error("[pow] The exponent must be a numeric value.");

    double exp_num = as_number(exponent);

    if (is_numeric(base))
    {
        double base_num = as_number(base);
        return NEW_NUM(pow(base_num, exp_num));
    }
    else if (IS_LIST(base))
    {
        list_t *input = AS_CLIST(base);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);

            if (!is_numeric(item))
                error("[pow] All elements in the base list must be numeric.");

            double val = as_number(item);
            double pow_val = pow(val, exp_num);
            Value v = NEW_NUM(pow_val);
            list_add(result, &v);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;

        return NEW_OBJ(list);
    }
    else
        error("[pow] The base argument must be a numeric value or a list of numeric values.");
    return NEW_NIL();
}

/**
 * Calculates the mean (average) of a list of numeric values.
 *
 * @param vm The virtual machine.
 * @param argc Number of arguments (expects exactly 1).
 * @param argv The arguments: a list of numeric values.
 * @return The mean (average) value as a number.
 */
Value pi_mean(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1)
        error("[mean] expects exactly one argument: a list of numeric values.");

    Value arg = argv[0];

    if (!IS_LIST(arg))
        error("[mean] expects a list of numeric values.");

    list_t *input = AS_CLIST(arg);

    if (input->size == 0)
        error("[mean] cannot compute mean of an empty list.");

    double sum = 0.0;

    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);

        if (!is_numeric(item))
            error("[mean] all elements in the list must be numeric.");

        sum += as_number(item);
    }

    double mean = sum / input->size;
    return NEW_NUM(mean);
}

/**
 * Alias for mean: calculates the average of a list of numeric values.
 */
Value pi_avg(vm_t *vm, int argc, Value *argv)
{
    return pi_mean(vm, argc, argv);
}

/**
 * Calculates the variance of a list of numeric values.
 * Expects a single argument: a list of numbers.
 * Returns the variance as a number.
 */
Value pi_var(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[var] expects a single argument: a list of numbers.");

    list_t *input = AS_CLIST(argv[0]);
    if (input->size == 0)
        error("[var] Cannot calculate variance of an empty list.");

    // Calculate mean
    double sum = 0.0;
    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
            error("[var] All elements in the list must be numeric.");

        sum += as_number(item);
    }
    double mean = sum / input->size;

    // Calculate variance
    double variance = 0.0;
    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        double diff = as_number(item) - mean;
        variance += diff * diff;
    }
    variance /= input->size;

    return NEW_NUM(variance);
}

/**
 * Calculates the standard deviation of a list of numeric values.
 * Expects a single argument: a list of numbers.
 * Returns the standard deviation as a number.
 */
Value pi_dev(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[dev] expects a single argument: a list of numbers.");

    list_t *input = AS_CLIST(argv[0]);
    if (input->size == 0)
        error("[dev] Cannot calculate standard deviation of an empty list.");

    // Calculate mean
    double sum = 0.0;
    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
            error("[dev] All elements in the list must be numeric.");

        sum += as_number(item);
    }
    double mean = sum / input->size;

    // Calculate variance
    double variance = 0.0;
    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        double diff = as_number(item) - mean;
        variance += diff * diff;
    }
    variance /= input->size;

    // Standard deviation is the square root of variance
    double stddev = sqrt(variance);

    return NEW_NUM(stddev);
}

// Comparison function for qsort to sort Values as numbers
static int compare_values(const void *a, const void *b)
{
    const Value *va = (const Value *)a;
    const Value *vb = (const Value *)b;

    double diff = as_number(*va) - as_number(*vb);
    if (diff < 0)
        return -1;
    if (diff > 0)
        return 1;
    return 0;
}

/**
 * Calculates the median of a list of numeric values.
 * Expects a single argument: a list of numbers.
 * Returns the median as a number.
 */
Value pi_median(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[median] expects a single argument: a list of numbers.");

    list_t *input = AS_CLIST(argv[0]);
    int size = input->size;
    if (size == 0)
        error("[median] Cannot calculate median of an empty list.");

    // Copy the values to a temporary array for sorting
    Value *copy = malloc(size * sizeof(Value));
    if (!copy)
        error("[median] Memory allocation failed.");

    for (int i = 0; i < size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
        {
            free(copy);
            error("[median] All elements in the list must be numeric.");
        }
        copy[i] = item;
    }

    // Sort the copy
    qsort(copy, size, sizeof(Value), compare_values);

    Value median;
    if (size % 2 == 1)
    {
        // Odd number of elements, take the middle one
        median = copy[size / 2];
    }
    else
    {
        // Even number of elements, average the two middle ones
        double mid1 = as_number(copy[size / 2 - 1]);
        double mid2 = as_number(copy[size / 2]);
        median = NEW_NUM((mid1 + mid2) / 2.0);
    }

    free(copy);
    return median;
}

/**
 * Returns the mode (most frequent numeric value) from a list of numbers.
 * Expects one argument: a list of numeric values.
 * Returns the mode as a numeric value.
 */
Value pi_mode(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[mode] expects a single argument: a list of numbers.");

    list_t *input = AS_CLIST(argv[0]);
    int size = input->size;

    if (size == 0)
        error("[mode] Cannot calculate mode of an empty list.");

    // Copy values to temporary array for sorting
    Value *copy = malloc(size * sizeof(Value));
    if (!copy)
        error("[mode] Memory allocation failed.");

    for (int i = 0; i < size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
        {
            free(copy);
            error("[mode] All elements in the list must be numeric.");
        }
        copy[i] = item;
    }

    // Sort the copy to group identical values together
    qsort(copy, size, sizeof(Value), compare_values);

    // Find the mode by counting frequencies
    int max_count = 1;
    int current_count = 1;
    Value mode = copy[0];

    for (int i = 1; i < size; i++)
    {
        if (as_number(copy[i]) == as_number(copy[i - 1]))
        {
            current_count++;
        }
        else
        {
            if (current_count > max_count)
            {
                max_count = current_count;
                mode = copy[i - 1];
            }
            current_count = 1;
        }
    }

    // Check last run
    if (current_count > max_count)
    {
        max_count = current_count;
        mode = copy[size - 1];
    }

    free(copy);
    return mode;
}

/**
 * Generates a random number between 0 and 1.
 *
 * @param vm The virtual machine instance.
 * @param argc The argument count; expected to be 0.
 * @param argv The argument values; expected to be empty.
 * @return A new Value containing the random number.
 */

Value pi_rand(vm_t *vm, int argc, Value *argv)
{
    static uint32_t state;
    static int initialized = 0;

    if (!initialized)
    {
        state = (uint32_t)time(NULL);
        initialized = 1;
    }

    return NEW_NUM(rand_num(&state));
}

/**
 * @brief Generates a list of random floating-point numbers between 0 and 1.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 1).
 * @param argv The arguments (expects one numeric argument for size).
 * @return A list of random numbers of length `size`.
 */
Value pi_rand_n(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !is_numeric(argv[0]))
        error("[rand_n] expects a single numeric argument representing the size.");

    int size = (int)as_number(argv[0]);
    if (size < 0)
        error("[rand_n] size must be non-negative.");

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < size; i++)
    {
        double r = (double)rand() / RAND_MAX; // random float between 0 and 1
        Value val = NEW_NUM(r);
        list_add(list, &val);
    }

    PiList *result = (PiList *)new_list(list);
    result->is_numeric = true;
    result->is_matrix = false;

    return NEW_OBJ(result);
}

/**
 * @brief Generates a random integer between min and max (inclusive).
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects exactly 2).
 * @param argv The arguments (expects two numeric arguments: min and max).
 * @return A random integer between min and max.
 */
Value pi_rand_i(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !is_numeric(argv[0]) || !is_numeric(argv[1]))
        error("[rand_i] expects two numeric arguments: min and max.");

    int min = (int)as_number(argv[0]);
    int max = (int)as_number(argv[1]);

    if (min > max)
        error("[rand_i] min must not be greater than max.");

    int range = max - min + 1;
    int r = rand() % range + min;

    return NEW_NUM(r);
}

/**
 * @brief Returns the minimum value in a list of numeric values.
 *
 * Accepts a single argument which must be a list of numeric values.
 * Returns the smallest number in the list.
 */
Value pi_min(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[min] expects a list of numeric values.");

    list_t *input = AS_CLIST(argv[0]);

    if (input->size == 0)
        error("[min] cannot operate on an empty list.");

    double min_val = 0;
    bool initialized = false;

    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
            error("[min] All elements in the list must be numeric.");

        double num = as_number(item);
        if (!initialized || num < min_val)
        {
            min_val = num;
            initialized = true;
        }
    }

    return NEW_NUM(min_val);
}

/**
 * @brief Returns the maximum value in a list of numeric values.
 *
 * Accepts a single argument which must be a list of numeric values.
 * Returns the largest number in the list.
 */
Value pi_max(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_LIST(argv[0]))
        error("[max] expects a list of numeric values.");

    list_t *input = AS_CLIST(argv[0]);

    if (input->size == 0)
        error("[max] cannot operate on an empty list.");

    double max_val = 0;
    bool initialized = false;

    for (int i = 0; i < input->size; i++)
    {
        Value item = *(Value *)list_getAt(input, i);
        if (!is_numeric(item))
            error("[max] All elements in the list must be numeric.");

        double num = as_number(item);
        if (!initialized || num > max_val)
        {
            max_val = num;
            initialized = true;
        }
    }

    return NEW_NUM(max_val);
}

/**
 * @brief Returns the absolute value of a number or a list of numbers.
 *
 * Accepts a single numeric value or a list of numeric values.
 * If a list is passed, returns a new list with the absolute value of each element.
 */
Value pi_abs(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        error("[abs] expects a numeric value or a list of numeric values.");

    Value arg = argv[0];

    if (is_numeric(arg))
        return NEW_NUM(fabs(as_number(arg)));

    else if (IS_LIST(arg))
    {
        list_t *input = AS_CLIST(arg);
        list_t *result = list_create(sizeof(Value));

        for (int i = 0; i < input->size; i++)
        {
            Value item = *(Value *)list_getAt(input, i);
            if (!is_numeric(item))
                error("[abs] All elements in the list must be numeric.");

            double _abs = fabs(as_number(item));
            Value val = NEW_NUM(_abs);
            list_add(result, &val);
        }

        PiList *list = (PiList *)new_list(result);
        list->is_numeric = true;
        list->is_matrix = false;
        return NEW_OBJ(list);
    }
    else
        error("[abs] expects a numeric value or a list of numeric values.");

    return NEW_NIL();
}

Value pi_log(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !is_numeric(argv[0]))
        error("[log] expects a single numeric argument.");

    double result = log(as_number(argv[0]));

    return NEW_NUM(result);
}