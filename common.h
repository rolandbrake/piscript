#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "screen.h"

#define PI 3.1415926535897932384626433832795
#define E 2.7182818284590452353602874713527


#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define ALLOCATE(type, count) (type *)calloc(count, sizeof(type))

/* RenderState is used to store the state of the rendering process
    such as whether or not it is currently running and the mutex and condition variable
    used to control access to the rendering process. */
typedef struct
{
    Screen *screen;
    bool render_flag;
    pthread_mutex_t render_mutex;
    pthread_cond_t render_cond;
} RenderState;

typedef uint8_t byte;
#define INIT_CAP 16 // initial capacity must not be zero

#define TARGET_FPS 60 // Target frames per second

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define False 0
#define True 1

double rand_num(uint32_t *state);
char *itos(int num);
void error(const char *format, ...);
#endif