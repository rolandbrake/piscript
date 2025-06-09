#ifndef PI_PLOT_H
#define PI_PLOT_H

#include "../pi_vm.h"
#include "../pi_value.h"

Value pi_pixel(vm_t *vm, int argc, Value *argv);
Value pi_line(vm_t *vm, int argc, Value *argv);
Value pi_draw(vm_t *vm, int argc, Value *argv);
Value pi_clear(vm_t *vm, int argc, Value *argv);
Value pi_circ(vm_t *vm, int argc, Value *argv);
Value pi_rect(vm_t *vm, int argc, Value *argv);
Value pi_poly(vm_t *vm, int argc, Value *argv);
Value pi_sprite(vm_t *vm, int argc, Value *argv);
Value pi_color(vm_t *vm, int argc, Value *argv);
#endif // PI_PLOT_H