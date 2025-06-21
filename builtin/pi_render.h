#ifndef PI_RENDER_H
#define PI_RENDER_H

// Include the VM and value system headers
#include "../pi_vm.h"
#include "../pi_value.h"

// A simple 3D vector structure
typedef struct
{
    float x, y, z; // 3D coordinates
} vec3d;

// Triangle structure used in rendering
typedef struct
{
    vec3d v[3];       // The three vertices of the triangle
    short color;      // Index to a color in the palette
    float brightness; // Lighting brightness (0.0 to 1.0) used for shading
} triangle;

// Loads a 3D model from an .obj file into a VM-compatible object
Value pi_load3d(vm_t *vm, int argc, Value *argv);

// Rotates the 3D model around x, y, and z axes
Value pi_rotate(vm_t *vm, int argc, Value *argv);

// Translates (moves) the model in 3D space
Value pi_translate(vm_t *vm, int argc, Value *argv);

// Scales the model in 3D space
Value pi_scale(vm_t *vm, int argc, Value *argv);

// Projects the 3D model to 2D screen space using a perspective projection matrix
Value pi_project(vm_t *vm, int argc, Value *argv);

// Renders the 3D model to the screen, optionally using shading
Value pi_render(vm_t *vm, int argc, Value *argv);

#endif // PI_RENDER_H
