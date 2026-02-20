#ifndef PI_RENDER_H
#define PI_RENDER_H

// Include the VM and value system headers
#include "../pi_vm.h"
#include "../pi_value.h"
#include "../common.h"

// Loads a 3D model from an .obj file into a VM-compatible object
Value pi_load3d(vm_t *vm, int argc, Value *argv);

// Rotates the 3D model around x, y, and z axes
Value pi_rotate3d(vm_t *vm, int argc, Value *argv);

// Translates (moves) the model in 3D space
Value pi_translate3d(vm_t *vm, int argc, Value *argv);

// Scales the model in 3D space
Value pi_scale3d(vm_t *vm, int argc, Value *argv);

// Projects the 3D model to 2D screen space using a perspective projection matrix
Value pi_project3d(vm_t *vm, int argc, Value *argv);

// Renders the 3D model to the screen, optionally using shading
Value pi_render3d(vm_t *vm, int argc, Value *argv);

#endif // PI_RENDER_H
