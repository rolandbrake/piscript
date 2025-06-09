#ifndef PI_FUN_H
#define PI_FUN_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value _pi_map(vm_t *vm, int argc, Value *argv);
Value pi_filter(vm_t *vm, int argc, Value *argv);
Value pi_reduce(vm_t *vm, int argc, Value *argv);
Value pi_find(vm_t *vm, int argc, Value *argv);

#endif // PI_FUN_H