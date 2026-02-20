#ifndef GC_H
#define GC_H
#include "pi_object.h"
#include "pi_vm.h"

void *reallocate(void *ptr, size_t o_size, size_t n_size);

void mark_constants(vm_t *vm);

void mark_value(Value val);
void mark_object(Object *obj);
void mark_roots(vm_t *vm);
void mark_globals(vm_t *vm);
void mark_iters(vm_t *vm);

void sweep(vm_t *vm);

void free_object(Object *obj);
void free_value(Value *val);

void run_gc(vm_t *vm);

#endif // GC_H