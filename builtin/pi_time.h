#ifndef PI_TIME_H
#define PI_TIME_H

#include <unistd.h>
#include <time.h>
#include <math.h>

#include "../pi_value.h"
#include "../pi_vm.h"

Value pi_sleep(vm_t *vm, int argc, Value *argv);
Value _pi_time(vm_t *vm, int argc, Value *argv);
#endif // PI_TIME_H