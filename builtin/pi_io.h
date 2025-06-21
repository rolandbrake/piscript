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

// open file and return file handle
Value pi_open(vm_t *vm, int argc, Value *argv);
// read from file and return string
Value pi_read(vm_t *vm, int argc, Value *argv);
// write to file
Value pi_write(vm_t *vm, int argc, Value *argv);

// seek in file and return if success (bool)
Value pi_seek(vm_t *vm, int argc, Value *argv);

// close file
Value pi_close(vm_t *vm, int argc, Value *argv);

#endif // PI_IO_H