#include "pi_frame.h"

/**
 * Creates a new frame with the given parameters.
 * @param pc The program counter to initialize the frame with.
 * @param sp The stack pointer to initialize the frame with.
 * @param bp The base pointer to initialize the frame with.
 * @param code The code the frame will be executing.
 * @param iters_top The current iterator stack top.
 * @param ip The instruction pointer to initialize the frame with.
 * @param fun_name The name of the function this frame is executing.
 * @return A newly allocated Frame struct.
 */
Frame *create_frame(int pc, int sp, int bp, list_t *code, int iters_top, int ip, Function *fn)
{
    Frame *frame = (Frame *)malloc(sizeof(Frame));

    frame->code = code;
    frame->pc = pc;
    frame->bp = bp;
    frame->sp = sp;
    frame->ip = ip;

    frame->iters_top = iters_top;

    frame->function = fn;

    return frame;
}

/**
 * Frees the memory allocated for a frame.
 *
 * @param frame The frame to free.
 */
void free_frame(Frame *frame)
{

    free(frame);
}
