#include "pi_mat.h"
#include "../list.h"

/**
 * @brief Returns the size of a matrix.
 *
 * Accepts a matrix as its first argument.
 * Returns a new list with two elements: the number of rows and the number of columns.
 */
Value pi_size(vm_t *vm, int argc, Value *argv)
{
    /* Check if the first argument is a matrix */
    if (argc != 1 || !IS_LIST(argv[0]))
        vm_error(vm, "Expected a matrix (list of lists)");

    PiList *list = AS_LIST(argv[0]);

    /* Check if the matrix is valid */
    if (!list->is_matrix)
        vm_error(vm, "Expected a matrix (list of lists)");

    int rows = list->rows;
    int cols = list->cols;

    /* Create a new list to store the size */
    list_t *_list = list_create(sizeof(Value));
    list_add(_list, &NEW_NUM(rows));
    list_add(_list, &NEW_NUM(cols));

    /* Create a new matrix to store the size */
    PiList *result = (PiList *)new_list(_list);

    result->is_numeric = true;
    result->is_matrix = true;

    result->rows = 1;
    result->cols = 2;

    return NEW_OBJ(result);
}

/**
 * @brief Creates a matrix of zeros with the given size.
 *
 * Accepts two numbers as its arguments: the number of rows and the number of columns.
 * Returns a new matrix with the given size filled with zeros.
 */
Value pi_zeros(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        vm_error(vm, "Expected two numbers (rows, cols)");

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

/**
 * @brief Creates a matrix of ones with the given size.
 *
 * Accepts two numbers as its arguments: the number of rows and the number of columns.
 * Returns a new matrix with the given size filled with ones.
 */
Value pi_ones(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        vm_error(vm, "Expected two numbers (rows, cols)");

    int rows = AS_NUM(argv[0]);
    int cols = AS_NUM(argv[1]);

    /* Create a new list to store the matrix */
    list_t *list = list_create(sizeof(Value));

    /* Iterate over the rows and columns to fill the matrix */
    for (int i = 0; i < rows; ++i)
    {
        list_t *row = list_create(sizeof(Value));
        for (int j = 0; j < cols; ++j)
            list_add(row, &NEW_NUM(1));
        list_add(list, &row);
    }

    /* Create a new matrix to store the result */
    PiList *mat = (PiList *)new_list(list);

    mat->is_numeric = true;
    mat->is_matrix = true;

    return NEW_OBJ(mat);
}

/**
 * @brief Creates an identity matrix with the given size.
 *
 * Accepts two numbers as its arguments: the number of rows and the number of columns.
 * Returns a new matrix with the given size filled with ones on the diagonal and zeros elsewhere.
 */
Value pi_eye(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]))
        vm_error(vm, "Expected two numbers (rows, cols)");

    int rows = AS_NUM(argv[0]);
    int cols = AS_NUM(argv[1]);

    /* Create a new list to store the matrix */
    list_t *list = list_create(sizeof(Value));

    /* Iterate over the rows and columns to fill the matrix */
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

    /* Create a new matrix to store the result */
    PiList *mat = (PiList *)new_list(list);

    mat->is_numeric = true;
    mat->is_matrix = true;

    return NEW_OBJ(mat);
}

/**
 * @brief Matrix multiplication.
 *
 * This function takes two matrices (lists of lists) and multiplies them.
 * The result is a new matrix with the same number of rows as the first
 * matrix and the same number of columns as the second matrix.
 *
 * @param list1 The first matrix (list of lists).
 * @param list2 The second matrix (list of lists).
 * @return The result of the matrix multiplication.
 */
Value pi_mult(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_LIST(argv[1]))
        vm_error(vm, "Expected two matrices (list of lists)");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        vm_error(vm, "Matrix multiplication requires numeric lists.");

    if (A->cols == -1 || B->cols == -1)
        vm_error(vm, "Matrix dimensions are not set properly.");

    if (A->cols != B->rows)
        vm_error(vm, "Matrix multiplication dimension mismatch.");

    int m = A->rows;
    int n = A->cols;
    int p = B->cols;

    list_t *result = list_create(sizeof(Value));

    /* Iterate over the rows of matrix A */
    for (int i = 0; i < m; i++)
    {
        Value *rowA_val = (Value *)list_getAt(A->items, i);
        list_t *rowA = as_list(*rowA_val);
        list_t *temp = list_create(sizeof(Value));

        /* Iterate over the columns of matrix B */
        for (int j = 0; j < p; j++)
        {
            double sum = 0.0;

            /* Iterate over the elements of row A and column B */
            for (int k = 0; k < n; k++)
            {
                /* Get A[i][k] */
                Value *a_val = (Value *)list_getAt(rowA, k);
                double a = as_number(*a_val);

                /* Get B[k][j] */
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
        vm_error(vm, "dot: Expected two numeric vectors (lists)");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        vm_error(vm, "dot: Vectors must be numeric");

    if (A->items->size != B->items->size)
        vm_error(vm, "dot: Vectors must be of same length");

    double sum = 0;
    for (int i = 0; i < A->items->size; i++)
    {
        double a = AS_NUM(*(Value *)list_getAt(A->items, i));
        double b = AS_NUM(*(Value *)list_getAt(B->items, i));
        sum += a * b;
    }

    return NEW_NUM(sum);
}


/**
 * @brief Computes the cross product of two 3D vectors.
 *
 * @param list1 The first vector (3D numeric vector).
 * @param list2 The second vector (3D numeric vector).
 * @return The cross product of the two input vectors.
 */
Value pi_cross(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2 || !IS_LIST(argv[0]) || !IS_LIST(argv[1]))
        vm_error(vm, "cross: Expected two 3D numeric vectors");

    PiList *A = AS_LIST(argv[0]);
    PiList *B = AS_LIST(argv[1]);

    if (!A->is_numeric || !B->is_numeric)
        vm_error(vm, "cross: Vectors must be numeric");

    if (A->items->size != 3 || B->items->size != 3)
        vm_error(vm, "cross: Only 3D vectors supported");

    // Extract components from the input vectors
    double a1 = AS_NUM(*(Value *)list_getAt(A->items, 0));
    double a2 = AS_NUM(*(Value *)list_getAt(A->items, 1));
    double a3 = AS_NUM(*(Value *)list_getAt(A->items, 2));

    double b1 = AS_NUM(*(Value *)list_getAt(B->items, 0));
    double b2 = AS_NUM(*(Value *)list_getAt(B->items, 1));
    double b3 = AS_NUM(*(Value *)list_getAt(B->items, 2));

    // Compute the cross product
    double x = a2 * b3 - a3 * b2;
    double y = a3 * b1 - a1 * b3;
    double z = a1 * b2 - a2 * b1;

    // Create the result vector
    list_t *items = list_create(sizeof(Value));
    list_add(items, &NEW_NUM(x));
    list_add(items, &NEW_NUM(y));
    list_add(items, &NEW_NUM(z));

    PiList *result = (PiList *)new_list(items);
    result->is_numeric = true;
    result->is_matrix = true;

    return NEW_OBJ(result);
}
/**
 * @brief Checks whether a given value is a matrix.
 *
 * A matrix is a list of lists where all sublists have the same length.
 * This function takes one argument and returns true if the argument is a matrix,
 * false otherwise.
 *
 * @param list The list value to check.
 * @return true if the input is a matrix, false otherwise.
 */
Value pi_isMat(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_LIST(argv[0]))
        vm_error(vm, "Expected a matrix (list of lists)");

    PiList *list = AS_LIST(argv[0]);
    // Check if the list is empty
    if (list->items->size == 0)
        return NEW_BOOL(false);

    int size = list->items->size;
    // Check if all sublists have the same length
    for (int i = 0; i < size; i++)
    {
        list_t *sublist = AS_CLIST(*(Value *)list_getAt(list->items, i));
        if (sublist->size != size)
            return NEW_BOOL(false);
    }

    return NEW_BOOL(true);
}
