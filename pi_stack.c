#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi_stack.h"
#include "common.h"

/**
 * Creates a new stack with the specified item size and initial capacity.
 *
 * This function allocates memory for the stack structure and initializes its
 * members. It sets the initial capacity to a fixed constant and allocates
 * memory for the data array to store the stack items.
 *
 * @param i_size The size of each item to be stored in the stack.
 * @return A pointer to the newly created stack.
 */
stack_t *stack_create(int i_size)
{
    stack_t *stack = (stack_t *)malloc(sizeof(stack_t));
    stack->i_size = i_size;
    stack->top = -1;
    stack->capacity = INIT_CAP; // Initial capacity
    stack->data = malloc(i_size * stack->capacity);

    return stack;
}

/**
 * Resizes the stack to a new capacity.
 *
 * This function updates the capacity of the stack to the specified
 * new capacity and reallocates memory for the stack's data array.
 * If the reallocation fails, it prints an error message and exits
 * the program.
 *
 * @param stack The stack to be resized.
 * @param new_cap The new capacity for the stack.
 */
void stack_expand(stack_t *stack, int new_cap)
{
    // Update the stack's capacity
    stack->capacity = new_cap;

    // Reallocate memory for the increased capacity
    stack->data = realloc(stack->data, stack->i_size * stack->capacity);
    if (stack->data == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to expand stack\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Adds an item to the stack. It increases top by 1 and copies the
 * given item to the stack.
 *
 * If the stack is full, it will double the capacity of the stack
 * before adding the item.
 *
 * @param stack The stack that the item should be added to.
 * @param item The item to add to the stack.
 */
void push(stack_t *stack, void *item)
{
    if (is_full(stack))
        stack_expand(stack, stack->capacity * 2); // Double the stack size
    stack->top++;
    memcpy((byte *)stack->data + (stack->top * stack->i_size), item, stack->i_size);    
}

/**
 * Removes an item from the stack. It decreases top by 1 and returns the item that
 * was at the top of the stack. If the stack is empty, it will return NULL and
 * print a stack underflow error message.
 */
void *pop(stack_t *stack)
{
    if (is_empty(stack))
    {
        printf("Stack underflow\n");
        return NULL;
    }
    void *item = malloc(stack->i_size);
    memcpy(item, (byte *)stack->data + (stack->top * stack->i_size), stack->i_size);
    stack->top--;
    return item;
}

/**
 * Returns the top item from the stack without modifying the stack pointer.
 *
 * @param stack The stack from which the top item should be returned.
 * @return A pointer to the top item in the stack. If the stack is empty, it
 *     returns NULL.
 */
void *top(stack_t *stack)
{
    if (is_empty(stack))
        return NULL;

    return (byte *)stack->data + (stack->top * stack->i_size);
}

// Stack is full when top is equal to the last index
int is_full(stack_t *stack)
{
    return stack->top == stack->capacity - 1;
}

/**
 * Checks if the stack is empty by comparing the top index to -1.
 *
 * @param stack The stack to check.
 * @return 1 if the stack is empty, 0 otherwise.
 */
int is_empty(stack_t *stack)
{
    return stack->top == -1;
}

/**
 * Retrieves an item from the stack at a specified index without removing it.
 *
 * @param stack The stack from which the item should be retrieved.
 * @param index The index of the item to retrieve.
 * @return A pointer to the item at the specified index. If the index is invalid,
 *         it returns NULL.
 */
void *stack_getAt(stack_t *stack, int index)
{
    // Check if the index is within valid range
    if (index < 0 || index > stack->top)
        return NULL; // Invalid index

    // Calculate the address of the item and return it
    return (byte *)stack->data + (index * stack->i_size);
}

int stack_size(stack_t *stack)
{
    return stack->top + 1; // Returns the number of elements in the stack
}

/**
 * Frees the memory allocated to the stack.
 *
 * This function is used to clean up the memory allocated to the stack
 * structure. It first frees the data array and then the stack structure
 * itself.
 *
 * @param stack The stack to be freed.
 */
void stack_free(stack_t *stack)
{
    free(stack->data); // Free the data array
    free(stack);       // Free the stack structure
}
