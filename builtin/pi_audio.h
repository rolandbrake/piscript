#ifndef PI_AUDIO_H
#define PI_AUDIO_H

#include <SDL2/SDL.h>
#include <pthread.h>
#include "../pi_value.h"
#include "../pi_object.h"
#include "../pi_vm.h"

typedef struct
{
    int frequency;
    int duration;
    int volume;
    WaveType wave_type;
} sound_params_t;

void init_audio(); // Initializes the sound thread

Value pi_sound(vm_t *vm, int argc, Value *argv);

// play a tone
Value pi_tone(vm_t *vm, int argc, Value *argv);

// play a melody of notes (freq, duration, waveform)
Value pi_melody(vm_t *vm, int argc, Value *argv);

// play sound with optional: channel, loop, start note, length
Value pi_play(vm_t *vm, int argc, Value *argv);

// stop sound from playing
Value pi_stop(vm_t *vm, int argc, Value *argv);

// returns whether a given sound object is currently playing
Value pi_isPlaying(vm_t *vm, int argc, Value *argv);

// returns channel index of a given sound object (-1 if not assigned)
Value pi_channel(vm_t *vm, int argc, Value *argv);

// sets default looping state on a sound object
Value pi_setLoop(vm_t *vm, int argc, Value *argv);

// resume sound from pause
Value pi_resume(vm_t *vm, int argc, Value *argv);

// pause sound from playing
Value pi_pause(vm_t *vm, int argc, Value *argv);

// stop all active audio playback/channels
void audio_stopAll(void);

// true when any mixer channel is currently playing
int audio_isPlaying(void);

// wait until channels stop playing (or timeout_ms reached if > 0)
void audio_waitForFinish(Uint32 timeout_ms);

#endif /* PI_AUDIO_H */
