#ifndef PI_SYS_H
#define PI_SYS_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_fps(vm_t *vm, int argc, Value *argv);
Value _pi_type(vm_t *vm, int argc, Value *argv);
Value pi_error(vm_t *vm, int argc, Value *argv);
Value pi_zen(vm_t *vm, int argc, Value *argv);
Value pi_cursor(vm_t *vm, int argc, Value *argv);
Value pi_mouse(vm_t *vm, int argc, Value *argv);
#endif // PI_SYS_H