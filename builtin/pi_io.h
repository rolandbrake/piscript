#ifndef PI_IO_H
#define PI_IO_H

#include <stdio.h>
#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_println(vm_t *vm, int argc, Value *argv);
Value pi_print(vm_t *vm, int argc, Value *argv);
Value pi_printf(vm_t *vm, int argc, Value *argv);

Value pi_text(vm_t *vm, int argc, Value *argv);

Value pi_key(vm_t *vm, int argc, Value *argv);
Value pi_input(vm_t *vm, int argc, Value *argv);

#endif // PI_IO_H