// #ifndef PI_AUDIO_H
// #define PI_AUDIO_H

// #include <stdio.h>
// #include "../pi_value.h"
// #include "../pi_vm.h"

// Value pi_sound(vm_t *vm, int argc, Value *argv);
// Value pi_music(vm_t *vm, int argc, Value *argv);

// #endif /* PI_AUDIO_H */

#ifndef PI_AUDIO_H
#define PI_AUDIO_H

#include <SDL2/SDL.h>
#include <pthread.h>
#include "../pi_value.h"
#include "../pi_vm.h"

// Waveform types
typedef enum
{
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_NOISE
} WaveType;

typedef struct
{
    int frequency;
    int duration;
    int volume;
    WaveType wave_type;
} sound_params_t;

void init_audio(); // Initializes the sound thread
Value pi_sound(vm_t *vm, int argc, Value *argv);
Value pi_music(vm_t *vm, int argc, Value *argv);

#endif /* PI_AUDIO_H */
