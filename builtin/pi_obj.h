#ifndef PI_OBJ_H
#define PI_OBJ_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_values(vm_t *vm, int argc, Value *argv);
Value pi_keys(vm_t *vm, int argc, Value *argv);
Value pi_clone(vm_t *vm, int argc, Value *argv);

#endif // PI_OBJ_H