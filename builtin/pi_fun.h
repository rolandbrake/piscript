#ifndef PI_FUN_H
#define PI_FUN_H

#include "../pi_value.h"
#include "../pi_vm.h"

// map function to map a function for each element of a list
Value _pi_map(vm_t *vm, int argc, Value *argv);

// filter function to filter a function for each element of a list
Value pi_filter(vm_t *vm, int argc, Value *argv);

// reduce function to reduce a function for each element of a list
Value pi_reduce(vm_t *vm, int argc, Value *argv);

// find function to find a function for each element of a list
Value pi_find(vm_t *vm, int argc, Value *argv);

#endif // PI_FUN_H