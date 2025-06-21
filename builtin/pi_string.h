#ifndef PI_STRING_H
#define PI_STRING_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_char(vm_t *vm, int argc, Value *argv);
Value pi_ord(vm_t *vm, int argc, Value *argv);
Value pi_trim(vm_t *vm, int argc, Value *argv);
Value pi_upper(vm_t *vm, int argc, Value *argv);
Value pi_lower(vm_t *vm, int argc, Value *argv);
Value pi_replace(vm_t *vm, int argc, Value *argv);
Value pi_isUpper(vm_t *vm, int argc, Value *argv);
Value pi_isLower(vm_t *vm, int argc, Value *argv);
Value pi_isDigit(vm_t *vm, int argc, Value *argv);
Value pi_isNumeric(vm_t *vm, int argc, Value *argv);
Value pi_isAlpha(vm_t *vm, int argc, Value *argv);
Value pi_isAlnum(vm_t *vm, int argc, Value *argv);
Value pi_split(vm_t *vm, int argc, Value *argv);

#endif // PI_STRING_H