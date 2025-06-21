#ifndef PI_COL_H
#define PI_COL_H
#include "../pi_value.h"
#include "../pi_vm.h"

// Removes and returns the last element or character from a list or string.
Value pi_pop(vm_t *vm, int argc, Value *argv);

// Appends one or more elements to the end of a list or string.
Value pi_push(vm_t *vm, int argc, Value *argv);

// Returns the last element or character from a list or string without removing it.
Value pi_peek(vm_t *vm, int argc, Value *argv);

// Checks if a list, string, or map is empty.
Value pi_empty(vm_t *vm, int argc, Value *argv);

// Sorts the elements of a list in ascending order.
Value pi_sort(vm_t *vm, int argc, Value *argv);

// Inserts a value at a specified index in a list or string.
Value pi_insert(vm_t *vm, int argc, Value *argv);

// Prepends one or more values to the beginning of a list or string.
Value pi_unshift(vm_t *vm, int argc, Value *argv);

// Removes and returns the element at a specified index from a list or string.
Value pi_remove(vm_t *vm, int argc, Value *argv);

// Appends a value to the end of a list or string (alias of push).
Value pi_append(vm_t *vm, int argc, Value *argv);

// Checks if a list contains a value, or if a map contains a key.
Value pi_contains(vm_t *vm, int argc, Value *argv);

// Returns the index of a value in a list or character in a string.
Value pi_indexOf(vm_t *vm, int argc, Value *argv);

// Reverses the elements of a list or characters of a string in place.
Value pi_reverse(vm_t *vm, int argc, Value *argv);

// Randomly shuffles the elements of a list.
Value pi_shuffle(vm_t *vm, int argc, Value *argv);

// Returns a shallow copy of a list or string.
Value pi_copy(vm_t *vm, int argc, Value *argv);

// Returns a slice of a list or string.
Value pi_slice(vm_t *vm, int argc, Value *argv);

// Returns the length of a list, string, or map.
Value pi_len(vm_t *vm, int argc, Value *argv);

// Returns a list of numbers in a specified range.
Value pi_range(vm_t *vm, int argc, Value *argv);

#endif // PI_COL_H