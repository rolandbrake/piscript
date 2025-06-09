// #include "pi_audio.h"
// #include <SDL2/SDL.h>
// #include <pthread.h>
// #include <math.h>
// #include <unistd.h>

// #define SAMPLE_RATE 44100
// #define AMPLITUDE 28000
// #define AUDIO_FORMAT AUDIO_S16SYS
// #define AUDIO_CHANNELS 1
// #define AUDIO_SAMPLES 4096

// // Global SDL Audio Device
// SDL_AudioDeviceID audio_device;

// // Sound parameters passed to the thread
// typedef struct
// {
//     int frequency;
//     int duration;
//     int volume;
// } sound_params_t;

// // Sound generator callback
// static void audio_callback(void *userdata, Uint8 *stream, int len)
// {
//     int frequency = ((sound_params_t *)userdata)->frequency;
//     int16_t *buffer = (int16_t *)stream;
//     int samples = len / sizeof(int16_t);
//     static int sample_pos = 0;

//     for (int i = 0; i < samples; i++)
//     {
//         buffer[i] = (int16_t)(AMPLITUDE * sin(2.0 * M_PI * frequency * sample_pos / SAMPLE_RATE));
//         sample_pos++;
//     }
// }

// // Function that runs in a separate thread to play the sound
// static void *sound_thread(void *arg)
// {
//     sound_params_t *params = (sound_params_t *)arg;

//     SDL_AudioSpec spec;
//     spec.freq = SAMPLE_RATE;
//     spec.format = AUDIO_FORMAT;
//     spec.channels = AUDIO_CHANNELS;
//     spec.samples = AUDIO_SAMPLES;
//     spec.callback = audio_callback;
//     spec.userdata = params;

//     if (SDL_Init(SDL_INIT_AUDIO) < 0)
//     {
//         fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
//         return NULL;
//     }

//     audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
//     if (!audio_device)
//     {
//         fprintf(stderr, "SDL Open Audio Device Failed: %s\n", SDL_GetError());
//         return NULL;
//     }

//     SDL_PauseAudioDevice(audio_device, 0); // Start playing
//     SDL_Delay(params->duration);           // Wait for the sound duration
//     SDL_CloseAudioDevice(audio_device);
//     // SDL_Quit();

//     free(params); // Free the sound parameters after use
//     return NULL;
// }

// Function to trigger sound in a separate thread
// Value pi_sound(vm_t *vm, int argc, Value *argv)
// {
//     if (argc < 3)
//     {
//         fprintf(stderr, "pi_sound requires 3 arguments: frequency, duration, and volume.\n");
//         return NEW_NIL();
//     }

//     int frequency = (int)as_number(argv[0]); // Frequency in Hz
//     int duration = (int)as_number(argv[1]);  // Duration in ms
//     int volume = (int)as_number(argv[2]);    // Volume (0-100)

//     sound_params_t *params = malloc(sizeof(sound_params_t));
//     params->frequency = frequency;
//     params->duration = duration;
//     params->volume = volume;

//     pthread_t thread_id;
//     pthread_create(&thread_id, NULL, sound_thread, (void *)params); // Create thread for sound

//     pthread_detach(thread_id); // Detach thread to let it run independently.

//     return NEW_NIL();
// }

// Value pi_sound(vm_t *vm, int argc, Value *argv)
// {
//     if (argc < 3)
//     {
//         fprintf(stderr, "pi_sound requires 3 arguments: frequency, duration, and volume.\n");
//         return NEW_NIL();
//     }

//     int frequency = (int)as_number(argv[0]); // Frequency in Hz
//     int duration = (int)as_number(argv[1]);  // Duration in ms
//     int volume = (int)as_number(argv[2]);    // Volume (0-100)

//     printf("Frequency: %d, Duration: %d, Volume: %d\n", frequency, duration, volume);

//     SDL_AudioSpec spec;
//     spec.freq = SAMPLE_RATE;
//     spec.format = AUDIO_FORMAT;
//     spec.channels = AUDIO_CHANNELS;
//     spec.samples = AUDIO_SAMPLES;
//     spec.callback = audio_callback;
//     spec.userdata = &frequency;

//     if (SDL_Init(SDL_INIT_AUDIO) < 0)
//     {
//         fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
//         return NEW_NIL();
//     }

//     audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
//     if (!audio_device)
//     {
//         fprintf(stderr, "SDL Open Audio Device Failed: %s\n", SDL_GetError());
//         return NEW_NIL();
//     }

//     SDL_PauseAudioDevice(audio_device, 0); // Start playing
//     SDL_Delay(duration);
//     // sleep(duration / 1000);
//     SDL_CloseAudioDevice(audio_device);
//     // SDL_Quit();

//     return NEW_NIL();
// }

// Function to play a music file using SDL_mixer
// Value pi_music(vm_t *vm, int argc, Value *argv)
// {
// if (argc < 1)
// {
//     fprintf(stderr, "pi_music requires 1 argument: file path.\n");
//     return NEW_NIL();
// }

// const char *file_path = AS_STRING(argv[0]);

// if (SDL_Init(SDL_INIT_AUDIO) < 0)
// {
//     fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
//     return NEW_NIL();
// }

// if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
// {
//     fprintf(stderr, "SDL_mixer Init Failed: %s\n", Mix_GetError());
//     return NEW_NIL();
// }

// Mix_Music *music = Mix_LoadMUS(file_path);
// if (!music)
// {
//     fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
//     return NEW_NIL();
// }

// Mix_PlayMusic(music, 1);

// // Wait for music to finish
// while (Mix_PlayingMusic())
// {
//     SDL_Delay(100);
// }

// Mix_FreeMusic(music);
// Mix_CloseAudio();
// SDL_Quit();

//     return NEW_NIL();
// }

// Value pi_sound(vm_t *vm, int argc, Value *argv)
// {
//     if (argc != 2)
//     {
//         fprintf(stderr, "pi_sound expects 2 arguments: frequency and duration\n");
//         return NEW_NIL();
//     }

//     int frequency = as_number(argv[0]);
//     int duration = as_number(argv[1]);

//     SDL_AudioSpec want, have;
//     SDL_AudioDeviceID dev;

//     if (SDL_Init(SDL_INIT_AUDIO) < 0)
//     {
//         fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
//         return NEW_NIL();
//     }

//     // Set up the audio specification we want
//     SDL_memset(&want, 0, sizeof(want));
//     want.freq = 44100;       // Sample rate (44.1 kHz)
//     want.format = AUDIO_S16; // 16-bit signed audio
//     want.channels = 1;       // Mono
//     want.samples = 2048;     // Buffer size
//     want.callback = NULL;    // We'll push audio data manually

//     // Open the audio device
//     dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
//     if (dev == 0)
//     {
//         fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
//         return NEW_NIL();
//     }

//     // Calculate the number of samples needed for the duration
//     int samples = (duration * have.freq) / 1000;
//     Uint8 *wave = (Uint8 *)malloc(samples * have.channels * sizeof(Uint16));

//     // Generate a square wave
//     for (int i = 0; i < samples; i++)
//     {
//         Uint16 value = ((i * frequency / have.freq) % 2) ? 32767 : -32768;
//         ((Uint16 *)wave)[i] = value;
//     }

//     // Queue the audio data
//     SDL_QueueAudio(dev, wave, samples * have.channels * sizeof(Uint16));
//     SDL_PauseAudioDevice(dev, 0); // Start playing

//     // Wait for the sound to finish playing
//     SDL_Delay(duration);
//     // sleep(duration / 1000);

//     // Clean up
//     SDL_CloseAudioDevice(dev);
//     free(wave);

//     return NEW_NIL();
// }

#include "pi_audio.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

// Constants
#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLES 4096
#define DEFAULT_VOLUME 100 // Fixed volume level

// SDL Audio Device
SDL_AudioDeviceID audio_device;

// Sound Queue
#define MAX_QUEUE_SIZE 32
sound_params_t sound_queue[MAX_QUEUE_SIZE];
int queue_start = 0, queue_end = 0;

// Synchronization
pthread_mutex_t sound_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sound_cond = PTHREAD_COND_INITIALIZER;

// Function to generate different waveforms
void generate_waveform(int16_t *buffer, int samples, sound_params_t *params)
{
    static int sample_pos = 0;

    for (int i = 0; i < samples; i++)
    {
        double t = (double)sample_pos / SAMPLE_RATE;
        double value = 0;

        switch (params->wave_type)
        {
        case WAVE_SINE:
            value = sin(2.0 * M_PI * params->frequency * t);
            break;
        case WAVE_SQUARE:
            value = (sin(2.0 * M_PI * params->frequency * t) >= 0) ? 1.0 : -1.0;
            break;
        case WAVE_TRIANGLE:
            value = 2.0 * fabs(fmod(params->frequency * t, 1.0) - 0.5) - 1.0;
            break;
        case WAVE_NOISE:
            value = ((rand() % 2001) - 1000) / 1000.0; // Random noise
            break;
        }

        buffer[i] = (int16_t)(AMPLITUDE * value * (DEFAULT_VOLUME / 100.0));
        sample_pos++;
    }
}

// Audio callback function
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    sound_params_t *params = (sound_params_t *)userdata;
    int16_t *buffer = (int16_t *)stream;
    int samples = len / sizeof(int16_t);

    generate_waveform(buffer, samples, params);
}

// Function to play a sound
void play_sound(int frequency, int duration, WaveType wave_type)
{
    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_FORMAT;
    spec.channels = AUDIO_CHANNELS;
    spec.samples = AUDIO_SAMPLES;
    spec.callback = audio_callback;

    sound_params_t params = {frequency, duration, DEFAULT_VOLUME, wave_type};
    spec.userdata = &params;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
        return;
    }

    audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!audio_device)
    {
        fprintf(stderr, "SDL Open Audio Device Failed: %s\n", SDL_GetError());
        return;
    }

    SDL_PauseAudioDevice(audio_device, 0);
    SDL_Delay(duration);
    SDL_CloseAudioDevice(audio_device);
}

// Sound thread that processes the queue
void *sound_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&sound_mutex);

        while (queue_start == queue_end)
        {
            pthread_cond_wait(&sound_cond, &sound_mutex);
        }

        sound_params_t sound = sound_queue[queue_start];
        queue_start = (queue_start + 1) % MAX_QUEUE_SIZE;

        pthread_mutex_unlock(&sound_mutex);

        play_sound(sound.frequency, sound.duration, sound.wave_type);
    }
    return NULL;
}

// Initialize the sound thread (call once in main)
void init_audio()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, sound_thread, NULL);
    pthread_detach(thread_id);
}

// Adds a sound request to the queue
Value pi_sound(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "pi_sound requires 3 arguments: frequency, duration, wave_type.\n");
        return NEW_NIL();
    }

    int frequency = (int)as_number(argv[0]);
    int duration = (int)as_number(argv[1]);
    WaveType wave_type = (WaveType)(int)as_number(argv[2]);

    pthread_mutex_lock(&sound_mutex);

    if ((queue_end + 1) % MAX_QUEUE_SIZE == queue_start)
    {
        fprintf(stderr, "Sound queue is full! Skipping sound request.\n");
        pthread_mutex_unlock(&sound_mutex);
        return NEW_NIL();
    }

    sound_queue[queue_end].frequency = frequency;
    sound_queue[queue_end].duration = duration;
    sound_queue[queue_end].volume = DEFAULT_VOLUME;
    sound_queue[queue_end].wave_type = wave_type;
    queue_end = (queue_end + 1) % MAX_QUEUE_SIZE;

    pthread_cond_signal(&sound_cond);
    pthread_mutex_unlock(&sound_mutex);

    return NEW_NIL();
}

Value pi_music(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_LIST(argv[0]))
    {
        fprintf(stderr, "pi_music requires a list of numbers: [freq1, dur1, wave1, freq2, dur2, wave2, ...]\n");
        return NEW_NIL();
    }

    list_t *music_list = AS_LIST(argv[0])->items;
    int length = list_size(music_list);

    if (length % 3 != 0)
    {
        fprintf(stderr, "pi_music list must have groups of 3 numbers (frequency, duration, wave_type).\n");
        return NEW_NIL();
    }

    for (int i = 0; i < length; i += 3)
    {
        int frequency = (int)as_number(*(Value *)list_getAt(music_list, i));
        int duration = (int)as_number(*(Value *)list_getAt(music_list, i + 1));
        WaveType wave_type = (WaveType)(int)as_number(*(Value *)list_getAt(music_list, i + 2));

        play_sound(frequency, duration, wave_type);
    }

    return NEW_NIL();
}

// #include "pi_audio.h"
// #include <stdlib.h>
// #include <math.h>
// #include <stdbool.h>
// #include <unistd.h>

// // Constants
// #define SAMPLE_RATE 44100
// #define AMPLITUDE 28000
// #define AUDIO_FORMAT AUDIO_S16SYS
// #define AUDIO_CHANNELS 1
// #define AUDIO_SAMPLES 4096

// // SDL Audio Device
// SDL_AudioDeviceID audio_device;

// // Sound Queue
// #define MAX_QUEUE_SIZE 32
// sound_params_t sound_queue[MAX_QUEUE_SIZE];
// int queue_start = 0, queue_end = 0;

// // Synchronization
// pthread_mutex_t sound_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t sound_cond = PTHREAD_COND_INITIALIZER;

// // Audio callback function
// void audio_callback(void *userdata, Uint8 *stream, int len)
// {
//     sound_params_t *params = (sound_params_t *)userdata;
//     int16_t *buffer = (int16_t *)stream;
//     int samples = len / sizeof(int16_t);
//     static int sample_pos = 0;

//     for (int i = 0; i < samples; i++)
//     {
//         buffer[i] = (int16_t)(AMPLITUDE * sin(2.0 * M_PI * params->frequency * sample_pos / SAMPLE_RATE));
//         sample_pos++;
//     }
// }

// // Function to play a sound
// void play_sound(int frequency, int duration)
// {
//     SDL_AudioSpec spec;
//     spec.freq = SAMPLE_RATE;
//     spec.format = AUDIO_FORMAT;
//     spec.channels = AUDIO_CHANNELS;
//     spec.samples = AUDIO_SAMPLES;
//     spec.callback = audio_callback;

//     sound_params_t params = {frequency, duration, 100};
//     spec.userdata = &params;

//     if (SDL_Init(SDL_INIT_AUDIO) < 0)
//     {
//         fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
//         return;
//     }

//     audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
//     if (!audio_device)
//     {
//         fprintf(stderr, "SDL Open Audio Device Failed: %s\n", SDL_GetError());
//         return;
//     }

//     SDL_PauseAudioDevice(audio_device, 0);
//     SDL_Delay(duration);
//     SDL_CloseAudioDevice(audio_device);
// }

// // Sound thread that processes the queue
// void *sound_thread(void *arg)
// {
//     while (1)
//     {
//         pthread_mutex_lock(&sound_mutex);

//         // Wait for a sound to be available in the queue
//         while (queue_start == queue_end)
//         {
//             pthread_cond_wait(&sound_cond, &sound_mutex);
//         }

//         // Get the next sound in the queue
//         sound_params_t sound = sound_queue[queue_start];
//         queue_start = (queue_start + 1) % MAX_QUEUE_SIZE;

//         pthread_mutex_unlock(&sound_mutex);

//         // Play the sound
//         play_sound(sound.frequency, sound.duration);
//     }
//     return NULL;
// }

// // Initialize the sound thread (call once in main)
// void init_audio()
// {
//     pthread_t thread_id;
//     pthread_create(&thread_id, NULL, sound_thread, NULL);
//     pthread_detach(thread_id);
// }

// // Adds a sound request to the queue
// Value pi_sound(vm_t *vm, int argc, Value *argv)
// {
//     if (argc < 3)
//     {
//         fprintf(stderr, "pi_sound requires 3 arguments: frequency, duration, and volume.\n");
//         return NEW_NIL();
//     }

//     int frequency = (int)as_number(argv[0]);
//     int duration = (int)as_number(argv[1]);
//     int volume = (int)as_number(argv[2]);

//     pthread_mutex_lock(&sound_mutex);

//     // Check if queue is full
//     if ((queue_end + 1) % MAX_QUEUE_SIZE == queue_start)
//     {
//         fprintf(stderr, "Sound queue is full! Skipping sound request.\n");
//         pthread_mutex_unlock(&sound_mutex);
//         return NEW_NIL();
//     }

//     // Add sound request to queue
//     sound_queue[queue_end].frequency = frequency;
//     sound_queue[queue_end].duration = duration;
//     sound_queue[queue_end].volume = volume;
//     queue_end = (queue_end + 1) % MAX_QUEUE_SIZE;

//     // Signal the sound thread that new data is available
//     pthread_cond_signal(&sound_cond);
//     pthread_mutex_unlock(&sound_mutex);

//     return NEW_NIL();
// }
