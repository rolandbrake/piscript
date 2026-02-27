#include "pi_audio.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL_atomic.h>

#define SFX_HW_CHANNELS 4

static Mix_Chunk *g_SFXChunks[SFX_HW_CHANNELS] = {0};
static int g_SFXIds[SFX_HW_CHANNELS] = {-1, -1, -1, -1};
static SDL_atomic_t g_SFXFinished[SFX_HW_CHANNELS];

/**
 * Function to generate different waveforms
 *
 * This function generates a waveform based on the given parameters.
 * The waveform is generated in a loop, with each iteration generating
 * a single sample of the waveform.
 *
 * @param buffer The buffer to fill with the waveform
 * @param samples The number of samples to generate
 * @param params The parameters to use for generating the waveform
 */
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
            // Generate a sine wave
            value = sin(2.0 * M_PI * params->frequency * t);
            break;
        case WAVE_SQUARE:
            // Generate a square wave
            value = (sin(2.0 * M_PI * params->frequency * t) >= 0) ? 1.0 : -1.0;
            break;
        case WAVE_TRIANGLE:
            // Generate a triangle wave
            value = 2.0 * fabs(fmod(params->frequency * t, 1.0) - 0.5) - 1.0;
            break;
        case SAWTOOTH:
            // Generate a sawtooth wave
            value = 2.0 * (t * params->frequency - floor(0.5 + t * params->frequency));
            break;
        case WAVE_NOISE:
            // Generate random noise
            value = ((rand() % 2001) - 1000) / 1000.0;
            break;
        }

        // Volume is 0-255
        buffer[i] = (int16_t)(AMPLITUDE * value * (params->volume / 255.0));
        sample_pos++;
    }
}

/**
 * Frees a slot for a sound effect to be played
 *
 * This function frees a slot in the sound effects system, which
 * allows the system to play a new sound effect.
 *
 * @param channel The channel to free
 */
static void SFX_freeSlot(int channel)
{
    if (channel < 0 || channel >= SFX_HW_CHANNELS)
        return;

    if (Mix_Playing(channel))
        Mix_HaltChannel(channel);

    Mix_Chunk *chunk = NULL;
    SDL_LockAudio();
    chunk = g_SFXChunks[channel];
    g_SFXChunks[channel] = NULL;
    g_SFXIds[channel] = -1;
    SDL_AtomicSet(&g_SFXFinished[channel], 0);
    SDL_UnlockAudio();

    if (chunk)
        Mix_FreeChunk(chunk);
}

/**
 * Called when a sound effect channel has finished playing
 *
 * This function is called by SDL_mixer when a sound effect channel has finished playing.
 * It is responsible for marking the channel as available and freeing any resources that
 * were allocated for the channel.
 *
 * @param channel The channel that has finished playing
 */
static void SFX_ChannelDone(int channel)
{
    if (channel < 0 || channel >= SFX_HW_CHANNELS)
    {
        // Invalid channel, do nothing
        return;
    }

    // Mark the channel as available
    SDL_AtomicSet(&g_SFXFinished[channel], 1);
}

/**
 * Called when a sound effect channel has finished playing.
 *
 * This function is responsible for freeing resources allocated for a channel when
 * a sound effect has finished playing.
 */
static void SFX_drainFinished(void)
{
    // Iterate over all sound effect channels and free any resources that
    // were allocated for a channel that has finished playing.
    for (int ch = 0; ch < SFX_HW_CHANNELS; ch++)
    {
        // Check if the channel has finished playing
        if (SDL_AtomicGet(&g_SFXFinished[ch]))
        {
            // Mark the channel as available
            SDL_AtomicSet(&g_SFXFinished[ch], 0);
            // Free resources allocated for the channel
            SFX_freeSlot(ch);
        }
    }
}

/**
 * Selects a free sound effect channel
 *
 * This function iterates over all sound effect channels and returns the first channel that is not currently playing a sound effect.
 *
 * @return The index of a free sound effect channel, or 0 if all channels are in use.
 */
static int SFX_selectChannel(void)
{
    // Iterate over all sound effect channels and return the first channel that is not currently playing a sound effect.
    for (int ch = 0; ch < SFX_HW_CHANNELS; ch++)
    {
        // Check if the channel is not currently playing a sound effect
        if (!Mix_Playing(ch))
        {
            // Return the index of the free channel
            return ch;
        }
    }

    // If all channels are in use, return 0
    return 0;
}

/**
 * Builds a Mix_Chunk for the given sound effect.
 *
 * This function allocates a Mix_Chunk and populates it with the samples
 * from the given sound effect. It also allocates a buffer to hold the
 * samples and populates it.
 *
 * @param vm The virtual machine that the function is being called from.
 * @param sfx The sound effect to build the Mix_Chunk for.
 * @param offset The offset into the sound effect's notes array to start
 *  generating samples from.
 * @param length The number of notes to generate samples for.
 * @return A pointer to the allocated Mix_Chunk.
 */
static Mix_Chunk *SFX_buildChunk(vm_t *vm, Sound *sfx, int offset, int length)
{
    int duration_per_note = (int)sfx->speed;
    if (duration_per_note <= 0)
        vm_error(vm, "[sfx] invalid sound speed");

    int total_samples = ((duration_per_note * length) * SAMPLE_RATE) / 1000;
    int buffer_size = total_samples * (int)sizeof(int16_t);

    // Allocate memory for the samples
    int16_t *samples = (int16_t *)SDL_malloc((size_t)buffer_size);
    if (!samples)
        vm_error(vm, "[sfx] failed to allocate audio buffer");

    int sample_offset = 0;
    for (int i = 0; i < length; i++)
    {
        Note *note = &sfx->notes[offset + i];
        int note_sample_count = (duration_per_note * SAMPLE_RATE) / 1000;

        // Generate a waveform for the note
        sound_params_t params = {
            .frequency = note->frequency,
            .duration = duration_per_note,
            .volume = note->volume,
            .wave_type = note->waveform,
        };

        generate_waveform(&samples[sample_offset], note_sample_count, &params);
        sample_offset += note_sample_count;
    }

    // Allocate memory for the Mix_Chunk
    Mix_Chunk *chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (!chunk)
    {
        SDL_free(samples);
        vm_error(vm, "[sfx] failed to allocate Mix_Chunk.");
    }

    // Populate the Mix_Chunk
    chunk->allocated = 1;
    chunk->abuf = (Uint8 *)samples;
    chunk->alen = (Uint32)buffer_size;
    chunk->volume = MIX_MAX_VOLUME;

    return chunk;
}

// Initialize the sound thread (call once in main)
void init_audio()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        error("SDL_Init failed: %s", SDL_GetError());

    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, 2048) == -1)
        error("Mix_OpenAudio failed: %s", Mix_GetError());

    Mix_AllocateChannels(MAX_CHANNELS);
    for (int ch = 0; ch < SFX_HW_CHANNELS; ch++)
        SDL_AtomicSet(&g_SFXFinished[ch], 0);
    Mix_ChannelFinished(SFX_ChannelDone);
}

/**
 * @brief Stops all sound effects.
 *
 * This function stops all sound effects that are currently playing.
 */
void audio_stopAll(void)
{
    // Stop all sound effects
    Mix_HaltChannel(-1);

    // Drain the sound effect channels
    SFX_drainFinished();

    // Free all resources allocated for sound effect channels
    for (int ch = 0; ch < SFX_HW_CHANNELS; ch++)
        SFX_freeSlot(ch);
}

/**
 * @brief Check if any sound effects are playing.
 *
 * This function checks if any sound effects are currently playing.
 * It also drains the sound effect channels to free up resources.
 *
 * @return True if any sound effects are playing, false otherwise.
 */
int audio_isPlaying(void)
{
    // Drain the sound effect channels to free up resources
    SFX_drainFinished();

    // Check if any sound effects are playing
    return Mix_Playing(-1) > 0;
}

/**
 * @brief Wait for all sound effects to finish playing.
 *
 * This function waits for all sound effects to finish playing. It can
 * be used to wait for a sound effect to finish before stopping the
 * sound thread.
 *
 * @param timeout_ms The timeout in milliseconds to wait for all sound
 * effects to finish playing. If 0, the function will wait indefinitely.
 */
void audio_waitForFinish(Uint32 timeout_ms)
{
    // Get the current time in milliseconds
    Uint32 start = SDL_GetTicks();

    // Wait for all sound effects to finish
    while (audio_isPlaying())
    {
        // Check if the timeout has been exceeded
        if (timeout_ms > 0 && (SDL_GetTicks() - start) >= timeout_ms)
            break;

        // Wait briefly before checking again
        SDL_Delay(5);
    }
}

/**
 * @brief Loads a sound object from a cartridge sound index.
 *
 * This function takes a single argument, a sound index, and loads the
 * corresponding sound from the cartridge. It allocates a new sound object
 * and initializes it with the data from the cartridge sound.
 *
 * @param sound_index Index into the cartridge sound list.
 * @return A Value containing a new sound object.
 */
Value pi_sound(vm_t *vm, int argc, Value *argv)
{

    if (argc < 1)
        vm_error(vm, "[sound] expects a sound index, or a file path string.");

    if (IS_NUM(argv[0]))
    {

        if (vm->cart == NULL || vm->cart->sounds == NULL)
            vm_error(vm, "[sound] no cartridge with sounds is loaded.");

        int index = (int)AS_NUM(argv[0]);
        if (index < 0 || index >= vm->cart->sfx_count)
            vm_error(vm, "[sound] sound index out of bounds.");

        Sound *sfx = &vm->cart->sounds[index];
        Mix_Chunk *chunk = SFX_buildChunk(vm, sfx, 0, NOTE_COUNT);

        ObjSound *sound = new_sound(chunk);
        sound->is_cart = true;
        sound->data = *sfx;
        sound->channel = -1;
        sound->looping = false;
        return NEW_OBJ(sound);
    }
    else if (IS_STR(argv[0]))
    {

        const char *path = as_string(argv[0]);
        Mix_Chunk *chunk = Mix_LoadWAV(path);
        if (!chunk)
        {
            char *buffer = (char *)malloc(strlen(path) + 1);
            snprintf(buffer, strlen(path) + 1, "Failed to load file '%s'", path);
            vm_error(vm, buffer);
        }

        ObjSound *sound = new_sound(chunk);
        sound->is_cart = false;
        sound->channel = -1;
        sound->looping = false;
        return NEW_OBJ(sound);
    }
    else
        vm_error(vm, "[sound] expects a sound index, or a file path string.");
}

/**
 * Play a sound object with optional looping.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1 or 2).
 * @param argv Arguments: [sound object, optional bool to loop]
 * @return `nil` on success, raises an error on failure.
 */
Value pi_play(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[play] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);

    int channel = -1;
    bool loop = sound->looping;
    int start = 0;
    int length = -1;

    if (argc >= 2)
    {
        if (!IS_BOOL(argv[1]))
            vm_error(vm, "[play] loop must be a boolean.");
        loop = AS_BOOL(argv[1]);
    }

    if (argc >= 3)
    {
        if (!IS_NUM(argv[2]))
            vm_error(vm, "[play] channel must be a number.");
        channel = (int)AS_NUM(argv[2]);
    }

    if (argc >= 4)
    {
        if (!IS_NUM(argv[3]))
            vm_error(vm, "[play] start must be a number.");
        start = (int)AS_NUM(argv[3]);
    }

    if (argc >= 5)
    {
        if (!IS_NUM(argv[4]))
            vm_error(vm, "[play] length must be a number.");
        length = (int)AS_NUM(argv[4]);
    }

    if (channel < -1 || channel >= MAX_CHANNELS)
        vm_error(vm, "[play] channel must be -1 or in range 0..31.");

    if (sound->is_cart)
    {
        int sound_len = (sound->data.length > 0 && sound->data.length <= NOTE_COUNT)
                            ? sound->data.length
                            : NOTE_COUNT;

        if (start < 0 || start >= sound_len)
            vm_error(vm, "[play] start must be within sound range.");

        int max_len = sound_len - start;
        int play_len = (length == -1) ? max_len : length;
        if (play_len < 0)
            vm_error(vm, "[play] length must be -1 or a positive value.");
        if (play_len > max_len)
            play_len = max_len;
        if (play_len <= 0)
            return NEW_NIL();

        if (sound->channel != -1 && Mix_Playing(sound->channel))
            Mix_HaltChannel(sound->channel);

        if (sound->chunk)
        {
            Mix_FreeChunk(sound->chunk);
            sound->chunk = NULL;
        }

        sound->chunk = SFX_buildChunk(vm, &sound->data, start, play_len);
        sound->loaded = true;
    }
    else if (start != 0 || length != -1)
        vm_error(vm, "[play] start/length are currently supported only for sounds loaded by sound(index).");

    if (!sound->chunk)
        vm_error(vm, "[play] sound has no loaded chunk.");

    int loops = loop ? -1 : 0;
    int _channel = Mix_PlayChannel(channel, sound->chunk, loops);

    if (_channel == -1)
        vm_errorf(vm, "[play] Failed to play sound: %s", Mix_GetError());

    sound->channel = _channel;
    sound->looping = loop;
    return NEW_NIL();
}

/**
 * Stop a sound object from playing.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [sound object]
 * @return `nil` on success, raises an error on failure.
 */
Value pi_stop(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[stop] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    if (sound->channel != -1 && Mix_Playing(sound->channel))
        Mix_HaltChannel(sound->channel);

    sound->channel = -1;
    sound->looping = false;

    return NEW_NIL();
}

Value pi_isPlaying(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[is_playing] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    if (sound->channel == -1)
        return NEW_BOOL(false);

    return NEW_BOOL(Mix_Playing(sound->channel) != 0);
}

Value pi_channel(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[channel] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    return NEW_NUM(sound->channel);
}

Value pi_setLoop(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND || !IS_BOOL(argv[1]))
        vm_error(vm, "[set_loop] expects (sound, bool).");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    sound->looping = AS_BOOL(argv[1]);
    return NEW_NIL();
}

Value pi_resume(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[resume] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    if (sound->channel != -1 && !Mix_Playing(sound->channel))
        Mix_Resume(sound->channel);

    return NEW_NIL();
}

Value pi_pause(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || argv[0].type != VAL_OBJ || AS_OBJ(argv[0])->type != OBJ_SOUND)
        vm_error(vm, "[pause] expects a sound object.");

    ObjSound *sound = (ObjSound *)AS_OBJ(argv[0]);
    if (sound->channel != -1 && Mix_Playing(sound->channel))
        Mix_Pause(sound->channel);

    return NEW_NIL();
}

Value pi_tone(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[tone] expects either a list or (freq, duration, waveform).");

    // ============================================================
    // MODE 1: RAW SAMPLE LIST  -> sampling primitive
    // ============================================================
    if (IS_LIST(argv[0]))
    {
        list_t *list = AS_LIST(argv[0])->items;
        int sample_count = list->size;

        if (sample_count <= 0)
            vm_error(vm, "[tone] sample list cannot be empty.");

        int buffer_size = sample_count * sizeof(int16_t);
        int16_t *samples = SDL_malloc(buffer_size);
        if (!samples)
            vm_error(vm, "Failed to allocate audio buffer.");

        for (int i = 0; i < sample_count; i++)
        {
            double value = as_number(LIST_AT(list, i));

            // clamp to [-1,1]
            if (value > 1.0)
                value = 1.0;
            if (value < -1.0)
                value = -1.0;

            samples[i] = (int16_t)(AMPLITUDE * value);
        }

        Mix_Chunk *chunk = SDL_malloc(sizeof(Mix_Chunk));
        if (!chunk)
        {
            SDL_free(samples);
            vm_error(vm, "Failed to allocate Mix_Chunk.");
        }

        chunk->allocated = 1;
        chunk->abuf = (Uint8 *)samples;
        chunk->alen = buffer_size;
        chunk->volume = MIX_MAX_VOLUME;

        ObjSound *sound = new_sound(chunk);
        sound->channel = -1;
        sound->looping = false;

        return NEW_OBJ(sound);
    }

    // ============================================================
    // MODE 2: freq, duration, waveform
    // ============================================================
    if (argc < 3 || !IS_NUM(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]))
        vm_error(vm, "[tone] expects (frequency, duration_ms, waveform).");

    int frequency = (int)AS_NUM(argv[0]);
    int duration = (int)AS_NUM(argv[1]);
    WaveType wave = (WaveType)(int)AS_NUM(argv[2]);

    if (frequency <= 0 || duration <= 0)
        vm_error(vm, "[tone] frequency and duration must be positive.");

    int sample_count = (duration * SAMPLE_RATE) / 1000;
    int buffer_size = sample_count * sizeof(int16_t);

    int16_t *samples = SDL_malloc(buffer_size);
    if (!samples)
        vm_error(vm, "Failed to allocate audio buffer.");

    sound_params_t params = {
        .frequency = frequency,
        .duration = duration,
        .volume = DEFAULT_VOLUME,
        .wave_type = wave,
    };

    generate_waveform(samples, sample_count, &params);

    Mix_Chunk *chunk = SDL_malloc(sizeof(Mix_Chunk));
    if (!chunk)
    {
        SDL_free(samples);
        vm_error(vm, "Failed to allocate Mix_Chunk.");
    }

    chunk->allocated = 1;
    chunk->abuf = (Uint8 *)samples;
    chunk->alen = buffer_size;
    chunk->volume = MIX_MAX_VOLUME;

    ObjSound *sound = new_sound(chunk);
    sound->channel = -1;
    sound->looping = false;

    return NEW_OBJ(sound);
}

Value pi_melody(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[melody] expects a list of notes (freq, duration, waveform).");

    if (IS_LIST(argv[0]))
    {
        list_t *list = AS_LIST(argv[0])->items;

        int size = list->size;
        if (size % 3 != 0)
            vm_error(vm, "[audio] List length must be a multiple of 3 (freq, duration, wave).");

        for (int i = 0; i < size; i++)
            if (!IS_NUM(LIST_AT(list, i)))
                vm_error(vm, "[melody] list values must be numbers.");

        int total_samples = 0;
        // Calculate total samples for all segments in the melody
        for (int i = 0; i < size; i += 3)
        {
            int duration = (int)as_number(LIST_AT(list, i + 1));
            total_samples += (duration * SAMPLE_RATE) / 1000;
        }

        int buffer_size = total_samples * sizeof(int16_t);
        int16_t *samples = SDL_malloc(buffer_size);
        if (!samples)
            vm_error(vm, "Failed to allocate audio buffer.");

        int sample_index = 0;
        // Generate waveform for each segment in the melody
        for (int i = 0; i < size; i += 3)
        {
            int frequency = (int)as_number(LIST_AT(list, i));
            int duration = (int)as_number(LIST_AT(list, i + 1));
            WaveType wave = (WaveType)(int)as_number(LIST_AT(list, i + 2));

            int sample_count = (duration * SAMPLE_RATE) / 1000;
            sound_params_t params = {
                .frequency = frequency,
                .duration = duration,
                .volume = DEFAULT_VOLUME,
                .wave_type = wave,
            };

            generate_waveform(&samples[sample_index], sample_count, &params);
            sample_index += sample_count;
        }

        Mix_Chunk *chunk = SDL_malloc(sizeof(Mix_Chunk));
        if (!chunk)
        {
            SDL_free(samples);
            vm_error(vm, "Failed to allocate Mix_Chunk.");
        }

        chunk->allocated = 1;
        chunk->abuf = (Uint8 *)samples;
        chunk->alen = buffer_size;
        chunk->volume = MIX_MAX_VOLUME;

        // Create and return a new sound object
        ObjSound *sound = new_sound(chunk);
        sound->is_cart = false;
        sound->channel = -1;
        sound->looping = false;

        return NEW_OBJ(sound);
    }
    else
        vm_error(vm, "[melody] expects a list of notes (freq, duration, waveform).");

    return NEW_NIL();
}
