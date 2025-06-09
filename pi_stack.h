#ifndef PI_STACK_H
#define PI_STACK_H

// Define macros for pushing and popping integers
#define PUSH_INT(stack, value) \
    do                         \
    {                          \
        int temp = value;      \
        push(stack, &temp);    \
    } while (0)

#define POP_INT(stack) *(int *)pop(stack)

// Define the structure for the stack
typedef struct
{
    void *data;   // Pointer to the array holding the stack items
    int i_size;   // Size of each individual item in the stack
    int top;      // Index of the top element in the stack (starts at -1 when empty)
    int capacity; // Total capacity of the stack (max number of items before expansion)
} stack_t;

// Function to create a new stack with a given item size
stack_t *stack_create(int i_size);

// Function to expand the stack's capacity to a new size
void stack_expand(stack_t *stack, int new_cap);

// Function to push an item onto the stack
void push(stack_t *stack, void *item);

// Function to pop the top item from the stack
// Returns a pointer to the popped item
void *pop(stack_t *stack);

// Function to return the top item from the stack without removing it
void *top(stack_t *stack);

// Function to check if the stack is empty
// Returns 1 if empty, 0 otherwise
int is_empty(stack_t *stack);

// Function to check if the stack is full
// Returns 1 if full, 0 otherwise
int is_full(stack_t *stack);

// Function to get an item from the stack at a specific index
// Returns a pointer to the item
void *stack_getAt(stack_t *stack, int index);

// Function to get the current number of items in the stack
int stack_size(stack_t *stack);

// Function to free the memory allocated for the stack
void stack_free(stack_t *stack);

#endif
