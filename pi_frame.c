#include "pi_frame.h"

Frame *create_frame(int pc, int sp, int bp, list_t *code, int iters_top, int ip)
{
    Frame *frame = (Frame *)malloc(sizeof(Frame));

    frame->code = code;
    frame->pc = pc;
    frame->bp = bp;
    frame->sp = sp;
    frame->ip = ip;

    frame->iters_top = iters_top;
    return frame;
}

void free_frame(Frame *frame)
{
    // list_free(frame->code);
    free(frame);
}
