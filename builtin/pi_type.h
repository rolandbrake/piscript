#ifndef PI_TYPE_H
#define PI_TYPE_H

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_isNum(vm_t *vm, int argc, Value *argv);
Value pi_isStr(vm_t *vm, int argc, Value *argv);
Value pi_isBool(vm_t *vm, int argc, Value *argv);
Value pi_isList(vm_t *vm, int argc, Value *argv);
Value pi_isMap(vm_t *vm, int argc, Value *argv);

Value pi_asNum(vm_t *vm, int argc, Value *argv);
Value pi_asStr(vm_t *vm, int argc, Value *argv);
Value pi_asBool(vm_t *vm, int argc, Value *argv);

#endif // PI_TYPE_H