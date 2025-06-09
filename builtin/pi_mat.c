#include "pi_mat.h"
#include "../list.h"

Value pi_size(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_LIST(argv[0]))
        error("Expected a matrix (list of lists)");

    PiList *list = AS_LIST(argv[0]);

    if (!list->is_matrix)
        error("Expected a matrix (list of lists)");

    int rows = list->rows;
    int cols = list->cols;

    list_t *_list = list_create(sizeof(Value));
    list_add(_list, &NEW_NUM(rows));
    list_add(_list, &NEW_NUM(cols));

    PiList *result = (PiList *)new_list(_list);

    result->is_numeric = true;
    result->is_matrix = true;

    result->rows = 1;
    result->cols = 2;

    return NEW_OBJ(result);
}

Value pi_zeros(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        error("Expected two numbers (rows, cols)");

    int rows = AS_NUM(argv[0]);
    int cols = AS_NUM(argv[1]);

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < rows; ++i)
    {
        list_t *row = list_create(sizeof(Value));
        for (int j = 0; j < cols; ++j)
            list_add(row, &NEW_NUM(0));
        list_add(list, &row);
    }

    PiList *mat = (PiList *)new_list(list);

    mat->is_numeric = true;
    mat->is_matrix = true;

    return NEW_OBJ(mat);
}

Value pi_ones(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        error("Expected two numbers (rows, cols)");

    int rows = AS_NUM(argv[0]);
    int cols = AS_NUM(argv[1]);

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < rows; ++i)
    {
        list_t *row = list_create(sizeof(Value));
        for (int j = 0; j < cols; ++j)
            list_add(row, &NEW_NUM(1));
        list_add(list, &row);
    }

    PiList *mat = (PiList *)new_list(list);

    mat->is_numeric = true;
    mat->is_matrix = true;

    return NEW_OBJ(mat);
}

Value pi_eye(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        error("Expected two numbers (rows, cols)");

    int rows = AS_NUM(argv[0]);
    int cols = AS_NUM(argv[1]);

    list_t *list = list_create(sizeof(Value));

    for (int i = 0; i < rows; ++i)
    {
        list_t *row = list_create(sizeof(Value));
        for (int j = 0; j < cols; ++j)
        {
            if (i == j)
                list_add(row, &NEW_NUM(1));
            else
                list_add(row, &NEW_NUM(0));
        }
        list_add(list, &row);
    }

    PiList *mat = (PiList *)new_list(list);

    mat->is_numeric = true;
    mat->is_matrix = true;

    return NEW_OBJ(mat);
}

Value pi_mult(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_LIST(argv[1]))
        error("Expected two matrices (list of lists)");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        error("Matrix multiplication requires numeric lists.");

    if (A->cols == -1 || B->cols == -1)
        error("Matrix dimensions are not set properly.");

    if (A->cols != B->rows)
        error("Matrix multiplication dimension mismatch.");

    int m = A->rows;
    int n = A->cols;
    int p = B->cols;

    list_t *result = list_create(sizeof(Value));

    for (int i = 0; i < m; i++)
    {
        Value *rowA_val = (Value *)list_getAt(A->items, i);
        list_t *rowA = as_list(*rowA_val);
        list_t *temp = list_create(sizeof(Value));

        for (int j = 0; j < p; j++)
        {
            double sum = 0.0;

            for (int k = 0; k < n; k++)
            {
                // Get A[i][k]
                Value *a_val = (Value *)list_getAt(rowA, k);
                double a = as_number(*a_val);

                // Get B[k][j]
                Value *rowB_val = (Value *)list_getAt(B->items, k);
                list_t *rowB = as_list(*rowB_val);
                Value *b_val = (Value *)list_getAt(rowB, j);
                double b = as_number(*b_val);

                sum += a * b;
            }

            list_add(temp, &NEW_NUM(sum));
        }

        list_add(result, &NEW_OBJ(new_list(temp)));
    }

    return NEW_OBJ(new_list(result));
}

Value pi_dot(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_LIST(argv[1]))
        error("dot: Expected two numeric vectors (lists)");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        error("dot: Vectors must be numeric");

    if (A->items->size != B->items->size)
        error("dot: Vectors must be of same length");

    double sum = 0;
    for (int i = 0; i < A->items->size; i++)
    {
        double a = AS_NUM(*(Value *)list_getAt(A->items, i));
        double b = AS_NUM(*(Value *)list_getAt(B->items, i));
        sum += a * b;
    }

    return NEW_NUM(sum);
}

Value pi_cross(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_LIST(argv[1]))
        error("cross: Expected two 3D numeric vectors");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        error("cross: Vectors must be numeric");

    if (A->items->size != 3 || B->items->size != 3)
        error("cross: Only 3D vectors supported");

    double a1 = AS_NUM(*(Value *)list_getAt(A->items, 0));
    double a2 = AS_NUM(*(Value *)list_getAt(A->items, 1));
    double a3 = AS_NUM(*(Value *)list_getAt(A->items, 2));

    double b1 = AS_NUM(*(Value *)list_getAt(B->items, 0));
    double b2 = AS_NUM(*(Value *)list_getAt(B->items, 1));
    double b3 = AS_NUM(*(Value *)list_getAt(B->items, 2));

    double x = a2 * b3 - a3 * b2;
    double y = a3 * b1 - a1 * b3;
    double z = a1 * b2 - a2 * b1;

    list_t *items = list_create(sizeof(Value));
    list_add(items, &NEW_NUM(x));
    list_add(items, &NEW_NUM(y));
    list_add(items, &NEW_NUM(z));

    PiList *result = (PiList *)new_list(items);
    result->is_numeric = true;
    result->is_matrix = true;

    return NEW_OBJ(result);
}

Value pi_isMat(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_LIST(argv[0]))
        error("Expected a matrix (list of lists)");

    return NEW_BOOL(AS_LIST(argv[0])->is_matrix);
}

