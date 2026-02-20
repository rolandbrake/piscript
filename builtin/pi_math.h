#ifndef PI_MATH_H
#define PI_MATH_H

#include "../pi_vm.h"
#include "../pi_value.h"

// Returns the floor of a number or each element in a list.
Value pi_floor(vm_t *vm, int argc, Value *argv);

// Returns the ceiling of a number or each element in a list.
Value pi_ceil(vm_t *vm, int argc, Value *argv);

// Rounds a number or each element in a list to the nearest integer.
Value pi_round(vm_t *vm, int argc, Value *argv);

// Sets the seed for the random number generator.
Value pi_seed(vm_t *vm, int argc, Value *argv);

// Returns a random float between 0 and 1.
Value pi_rand(vm_t *vm, int argc, Value *argv);

// Returns a list of random floats of specified size.
Value pi_rand_n(vm_t *vm, int argc, Value *argv);

// Returns the square root of a number or each element in a list.
Value pi_sqrt(vm_t *vm, int argc, Value *argv);

// Returns the sine of a number or each element in a list (input in radians).
Value pi_sin(vm_t *vm, int argc, Value *argv);

// Returns the cosine of a number or each element in a list (input in radians).
Value pi_cos(vm_t *vm, int argc, Value *argv);

// Returns the tangent of a number or each element in a list (input in radians).
Value pi_tan(vm_t *vm, int argc, Value *argv);

// Returns the arcsine of a number or each element in a list (output in radians).
Value pi_asin(vm_t *vm, int argc, Value *argv);

// Returns the arccosine of a number or each element in a list (output in radians).
Value pi_acos(vm_t *vm, int argc, Value *argv);

// Returns the arctangent of a number or each element in a list (output in radians).
Value pi_atan(vm_t *vm, int argc, Value *argv);

// Converts radians to degrees for a number or each element in a list.
Value pi_deg(vm_t *vm, int argc, Value *argv);

// Converts degrees to radians for a number or each element in a list.
Value pi_rad(vm_t *vm, int argc, Value *argv);

// Returns the sum of all numbers in a list.
Value pi_sum(vm_t *vm, int argc, Value *argv);

// Returns e raised to the power of a number or each element in a list.
Value pi_exp(vm_t *vm, int argc, Value *argv);

// Returns the base-2 logarithm of a number or each element in a list.
Value pi_log2(vm_t *vm, int argc, Value *argv);

// Returns the base-10 logarithm of a number or each element in a list.
Value pi_log10(vm_t *vm, int argc, Value *argv);

// Returns a number raised to the power of another number.
Value pi_pow(vm_t *vm, int argc, Value *argv);

// Returns the absolute value of a number or each element in a list.
Value pi_abs(vm_t *vm, int argc, Value *argv);

// Returns the mean (average) of a list of numbers.
Value pi_mean(vm_t *vm, int argc, Value *argv);

// Returns the average of a list of numbers (alias for mean).
Value pi_avg(vm_t *vm, int argc, Value *argv);

// Returns the variance of a list of numbers.
Value pi_var(vm_t *vm, int argc, Value *argv);

// Returns the standard deviation of a list of numbers.
Value pi_dev(vm_t *vm, int argc, Value *argv);

// Returns the median value of a list of numbers.
Value pi_median(vm_t *vm, int argc, Value *argv);

// Returns the mode (most frequent value) of a list of numbers.
Value pi_mode(vm_t *vm, int argc, Value *argv);

// Returns the maximum value from a list of numbers.
Value pi_max(vm_t *vm, int argc, Value *argv);

// Returns the minimum value from a list of numbers.
Value pi_min(vm_t *vm, int argc, Value *argv);

#endif // PI_MATH_H