#ifndef PI_IMG_H
#define PI_IMG_H

#include <SDL2/SDL.h>
#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_image(vm_t *vm, int argc, Value *argv);

#endif // PI_IMG_H