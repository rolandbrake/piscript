#ifndef PI_SHELL_H
#define PI_SHELL_H
#include "pi_vm.h"

#define MAX_SHELL_INPUT_LENGTH 64
#define HISTORY_MAX 64
#define LINE_MAX 256

#define LOGO_W 50
#define LOGO_H 12
#define LOGO_X 4
#define LOGO_Y 4

extern char history[HISTORY_MAX][LINE_MAX];
extern int history_count; // total stored
extern int history_pos;   // current navigation index

// Options for variadic shell functions
typedef enum
{
    SHELL_END = 0,  // End of options
    SHELL_COLOR,    // The value is an int (color index)
    REPL_MODE,      // The value is a bool (true for REPL mode)
    SHELL_CURSOR_X, // The value is an int (cursor x position)
    SHELL_CURSOR_Y  // The value is an int (cursor y position)
} shell_opt;

// I/O function pointer types
typedef void (*out_fn)(const char *s, ...);
typedef void (*in_fn)(char *buffer, int size, ...);
typedef void (*clear_fn)(Color color);

// Shell context
typedef struct
{
    out_fn out;
    in_fn in;
    clear_fn clear;
    vm_t *vm;
} shell_io_t;

extern shell_io_t *shell_io;
extern bool shell_home_visible;

void shell_run(shell_io_t *io);
void shell_stop(void);

// Draw the PiScript logo at the specified (x, y) position
void draw_logo(int x, int y);
void draw_logoWave(int x, int y, double phase);

#endif // PI_SHELL_H
