#ifndef PI_IMG_H
#define PI_IMG_H

#include <SDL2/SDL.h>
#include "../pi_value.h"
#include "../pi_vm.h"

// Load an image from a file and convert it to palette-based image object
Value pi_image(vm_t *vm, int argc, Value *argv);

// Crop a region from the image (returns a new image)
Value pi_crop(vm_t *vm, int argc, Value *argv);

// Resize the image to a new width and height (returns a new image)
Value pi_resize(vm_t *vm, int argc, Value *argv);

// Scale the image by floating-point scale factors (sx, sy)
Value pi_scale2d(vm_t *vm, int argc, Value *argv);

// Translate image pixels by dx, dy (out-of-bound areas filled with transparency)
Value pi_tran2d(vm_t *vm, int argc, Value *argv);

// Rotate the image by 90, 180, or 270 degrees
Value pi_rotate2d(vm_t *vm, int argc, Value *argv);

// Optional: Create a copy of the image (deep copy)
Value pi_copy2d(vm_t *vm, int argc, Value *argv);

// Flip the image horizontally and/or vertically
Value pi_flip(vm_t *vm, int argc, Value *argv);

// Display the image object onto the screen at optional (x, y) position
Value pi_rend2d(vm_t *vm, int argc, Value *argv);

// // Optional: Get color and alpha at pixel (x, y)
// Value pi_get2d(vm_t *vm, int argc, Value *argv);

// // Optional: Set color and alpha at pixel (x, y)
// Value pi_set2d(vm_t *vm, int argc, Value *argv);

#endif // PI_IMG_H
