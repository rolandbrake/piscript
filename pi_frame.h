#ifndef PI_FRAME_H
#define PI_FRAME_H

#include "list.h"

// Forward declare Function to avoid circular include
typedef struct Function Function;

typedef struct
{
    int pc; // program counter
    int sp; // stack pointer
    int bp; // base pointer
    int ip; // instruction pointer

    list_t *code; // list of instructions

    int iters_top; // to track the state of iterators stack

    Function *function;
} Frame;

Frame *create_frame(int pc, int sp, int bp, list_t *code, int iters_top, int ip, Function *fn);
void free_frame(Frame *frame);

#endif