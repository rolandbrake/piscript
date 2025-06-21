#ifndef PI_VM_H
#define PI_VM_H

#include <pthread.h>

#include "pi_compiler.h"
#include "pi_table.h"
#include "pi_stack.h"
#include "list.h"
#include "pi_object.h"
#include "screen.h"
#include "pi_frame.h"


#define STACK_MAX 1024 // max stack size
#define ITER_MAX 256   // max iterator stack size

#define RUN_STEPS 1024 // max number of instructions to run

// #define NEXT_GC (1024 * 1024)
#define NEXT_GC 1024

typedef struct
{
    int pc; // Program Counter: Points to the current instruction being executed.
    int sp; // Stack Pointer: Tracks the top of the stack.
    int bp; // Base Pointer: Used for managing function call frames.
    int ip; // Instruction Pointer: Points to the current instruction being executed.

    Value stack[STACK_MAX]; // Operand stack for storing temporary values and function calls.

    // stack_t *frames; // Call stack frames, storing function call contexts.

    Frame *frames[STACK_MAX]; // Call stack frames, storing function call contexts.
    int frame_sp;

    list_t *code;      // PiList of bytecode instructions.
    list_t *constants; // PiList of constant values used in the program.
    list_t *names;     // PiList of variable/function names for identifier lookup.

    table_t *globals; // Hash table storing global variables.

    Object *objects; // Linked list of dynamically allocated objects (for garbage collection).

    Object *iters[STACK_MAX]; // Iterator stack to support loops and iteration constructs.
    int iter_sp;              // Iterator Stack Pointer: Tracks the top of the iterator stack.

    // UpValue *openUpvalues[STACK_MAX]; // Stack of open upvalues used in nested functions.
    // int upvalue_sp;

    UpValue *openUpvalues; // linked list of open upvalues used in nested functions.

    Screen *screen; // Pointer to the screen object, used for rendering graphics.

    bool running;         // Flag indicating whether the VM is currently executing code.
    pthread_mutex_t lock; // Mutex for thread synchronization.

    double fps; // Frames per second (used for performance measurement in graphical applications).

    Object *function;

    int counter;

    list_t *instrs; // PiList of instruction metadata

    int next_gc; // Next garbage collection threshold

    int obj_count;

} vm_t;

vm_t *init_vm(compiler_t *comp, Screen *screen);

Object *add_obj(vm_t *vm, Object *obj);
void run(vm_t *vm);

void push_frame(vm_t *vm, Frame *frame);
Frame *pop_frame(vm_t *vm);

void free_vm(vm_t *vm);

#endif // PI_VM_H