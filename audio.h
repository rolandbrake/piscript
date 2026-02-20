// audio.h
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// Constants
#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLES 4096
#define DEFAULT_VOLUME 100 // Fixed volume level

#define MAX_CHANNELS 32

#define NOTE_COUNT 32

typedef uint8_t WaveType;
enum
{
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    SAWTOOTH,
    WAVE_NOISE
};

typedef struct
{
    uint16_t frequency; // Frequency in Hz
    uint8_t volume;     // Volume (0-255)
    WaveType waveform;  // waveform type
} Note;

typedef struct
{
    uint16_t speed;         // ticks per note
    uint16_t length;        // number of notes [0..NOTE_COUNT]
    Note notes[NOTE_COUNT]; // pointer to array of notes
} Sound;

#endif // AUDIO_H
