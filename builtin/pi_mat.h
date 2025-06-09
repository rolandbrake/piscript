#ifndef PI_MAT_H
#define PI_MAT_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_size(vm_t *vm, int argc, Value *argv);

Value pi_zeros(vm_t *vm, int argc, Value *argv);
Value pi_ones(vm_t *vm, int argc, Value *argv);
Value pi_eye(vm_t *vm, int argc, Value *argv);

Value pi_mult(vm_t *vm, int argc, Value *argv);
Value pi_dot(vm_t *vm, int argc, Value *argv);
Value pi_cross(vm_t *vm, int argc, Value *argv);

// check if the giving list is a matrix
Value pi_isMat(vm_t *vm, int argc, Value *argv);

#endif // PI_MAT_H