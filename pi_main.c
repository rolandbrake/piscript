#ifdef __EMSCRIPTEN__

// Emscripten-specific includes and code

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "screen.h"
#include "common.h"
#include "pi_lex.h"
#include "pi_parser.h"
#include "pi_stack.h"
#include "pi_compiler.h"
#include "pi_vm.h"
#include "./builtin/pi_audio.h"

#ifndef TARGET_FPS
#define TARGET_FPS 60
#endif

Screen *screen;
vm_t *vm;
SDL_Event event;
int frame_count = 0;
double fps = 0.0;
Uint32 last_time;
clock_t start_time;
char *source = NULL;

bool paused = false;

static void web_error_handler(const char *message, int line, int column)
{
    if (line >= 0 && column >= 0)
        fprintf(stderr, "[PiScript Error] at line %d, column %d: %s\n",
                line, column, message);
    else
        fprintf(stderr, "[PiScript Error] %s\n", message);

    if (vm)
    {
        vm->running = false;
        paused = false;
    }
}

void main_loop()
{
    Uint32 frame_start = SDL_GetTicks();

    Uint32 current_time = SDL_GetTicks();
    Uint32 delta_time = current_time - last_time;
    last_time = current_time;

    // Run the main loop
    if (vm && vm->running)
        run(vm);

    else if (!paused)
    {
        clock_t end_time = clock();
        double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
        printf("Execution Time: %.4f ms\n", time_taken);

        EM_ASM({
            if (typeof onExecutionFinished == 'function')
                onExecutionFinished();
        });

        emscripten_cancel_main_loop();
    }

    // emscripten_sleep(0);
}

EMSCRIPTEN_KEEPALIVE
void set_source(const char *_source)
{
    if (source)
        free(source);
    source = strdup(_source);
}

EMSCRIPTEN_KEEPALIVE
void stop_execution(void)
{

    if (!vm || !screen || !screen->renderer)
        return;
    printf("Stopping execution from [c]\n");
    vm->running = false;
    paused = false;
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
    printf("Execution Time: %.4f ms\n", time_taken);

    EM_ASM({
        if (typeof onExecutionFinished == 'function')
            onExecutionFinished();
    });

    audio_stopAll();
    emscripten_cancel_main_loop();
}

EMSCRIPTEN_KEEPALIVE
void pause_execution(void)
{
    vm->running = false;

    paused = true;
    // vm->paused = true;
}

EMSCRIPTEN_KEEPALIVE
void resume_execution(void)
{
    if (vm)
    {
        vm->running = true;
        paused = false;
    }
}

EMSCRIPTEN_KEEPALIVE
void _init_audio(void)
{
    init_audio();
}

EM_JS(void, js_load_sprites_to_memory, (), {
    const saved = localStorage.getItem('savedSprites');
    if (!saved)
        return;

    const sprites = JSON.parse(saved);
    const spriteCount = sprites.length;

    // Compute buffer size: for each sprite, 2x uint16 + pixel data
    let totalBytes = 0;
    for (let i = 0; i < spriteCount; i++)
    {
        const s = sprites[i];
        totalBytes += 4 + (s.width * s.height);
    }
    const buffer = _malloc(totalBytes);

    let offset = buffer;
    for (let i = 0; i < spriteCount; i++)
    {
        const s = sprites[i];
        const width = s.width;
        const height = s.height;
        const pixels = s.pixels;
        const pixelCount = width * height;

        // Write as bytes to avoid misaligned HEAPU16 writes when offset is odd.
        Module.HEAPU8[offset] = width & 0xff;
        Module.HEAPU8[offset + 1] = (width >> 8) & 0xff;
        Module.HEAPU8[offset + 2] = height & 0xff;
        Module.HEAPU8[offset + 3] = (height >> 8) & 0xff;
        offset += 4;

        if (Array.isArray(pixels) && pixels.length > 0 && Array.isArray(pixels[0]))
        {
            let k = 0;
            for (let y = 0; y < height; y++)
            {
                const row = pixels[y] || [];
                for (let x = 0; x < width; x++)
                {
                    const v = Number(row[x] ?? 0);
                    Module.HEAPU8[offset + k] = (Number.isFinite(v) ? v : 0) & 0xff;
                    k++;
                }
            }
        }
        else
        {
            for (let j = 0; j < pixelCount; j++)
            {
                const v = Number((pixels && pixels[j]) ?? 0);
                Module.HEAPU8[offset + j] = (Number.isFinite(v) ? v : 0) & 0xff;
            }
        }
        offset += pixelCount;
    }

    _load_sprites_from_buffer(buffer, spriteCount);
    _free(buffer);
});

EM_JS(void, js_load_sfx_to_memory, (), {
    const saved = localStorage.getItem('savedSfx');
    if (!saved)
        return;

    const sfx = JSON.parse(saved);
    const sfxCount = sfx.length;
    const NOTE_COUNT = 32;
    const BASE_FREQUENCY = 110;
    const UI_VOLUME_MAX = 15;
    const STORED_VOLUME_MAX = 255;

    // Each sound: uint16 speed + NOTE_COUNT * (uint8 volume + uint16 frequency + uint8 waveform)
    const bytesPerNote = 4;
    const bytesPerSfx = 2 + (NOTE_COUNT * bytesPerNote);
    const totalBytes = sfxCount * bytesPerSfx;

    const buffer = _malloc(totalBytes);
    let offset = buffer;

    for (let i = 0; i < sfxCount; i++)
    {
        const s = sfx[i];
        const speed = s.speed || 0;
        Module.HEAPU8[offset] = speed & 0xff;
        Module.HEAPU8[offset + 1] = (speed >> 8) & 0xff;
        offset += 2;

        const notes = s.notes || [];
        for (let n = 0; n < NOTE_COUNT; n++)
        {
            const note = notes[n] || {};
            let frequency = 0;
            if (typeof note.frequency === 'number')
                frequency = Math.round(note.frequency);
            else if (typeof note.pitch === 'number')
                frequency = Math.round(BASE_FREQUENCY * Math.pow(2, note.pitch / 12));

            let volume = 0;
            if (typeof note.volume === 'number')
            {
                if (note.volume <= 1 && !Number.isInteger(note.volume))
                    volume = Math.round(note.volume * STORED_VOLUME_MAX);
                else if (note.volume <= UI_VOLUME_MAX)
                    volume = Math.round((note.volume / UI_VOLUME_MAX) * STORED_VOLUME_MAX);
                else
                    volume = Math.round(note.volume);
            }

            let waveform = 0;
            if (typeof note.waveform === 'number')
                waveform = note.waveform;
            else if (typeof note.wave === 'number')
                waveform = note.wave;
            else if (typeof note.wave === 'string')
            {
                const waveMap = {sine : 0, square : 1, triangle : 2, sawtooth : 3, noise : 4};
                waveform = waveMap[note.wave] ?? 0;
            }

            frequency = Math.max(0, Math.min(0xffff, frequency));
            volume = Math.max(0, Math.min(0xff, volume));
            waveform = Math.max(0, Math.min(4, waveform));

            Module.HEAPU8[offset] = volume;
            Module.HEAPU8[offset + 1] = frequency & 0xff;
            Module.HEAPU8[offset + 2] = (frequency >> 8) & 0xff;
            Module.HEAPU8[offset + 3] = waveform;
            offset += bytesPerNote;
        }
    }

    _load_sfx_from_buffer(buffer, sfxCount);
    _free(buffer);
});

static void free_cart_sprites(Cart *cart)
{
    if (!cart)
        return;
    if (cart->sprites)
    {
        for (int i = 0; i < cart->spr_count; i++)
            if (cart->sprites[i].pixels)
                free(cart->sprites[i].pixels);
        free(cart->sprites);
        cart->sprites = NULL;
        cart->spr_count = 0;
    }
}

static void free_cart_sfx(Cart *cart)
{
    if (!cart)
        return;
    if (cart->sounds)
    {
        free(cart->sounds);
        cart->sounds = NULL;
        cart->sfx_count = 0;
    }
}

EMSCRIPTEN_KEEPALIVE
void load_sprites_from_buffer(const uint8_t *buffer, int sprite_count)
{
    if (!vm)
        return;
    if (!vm->cart)
        vm->cart = (Cart *)calloc(1, sizeof(Cart));
    if (!vm->cart)
        return;

    free_cart_sprites(vm->cart);

    vm->cart->spr_count = (uint16_t)sprite_count;
    if (sprite_count <= 0)
        return;

    vm->cart->sprites = (Sprite *)calloc(sprite_count, sizeof(Sprite));
    if (!vm->cart->sprites)
        return;

    const uint8_t *p = buffer;
    for (int i = 0; i < sprite_count; i++)
    {
        uint16_t width = (uint16_t)(p[0] | (p[1] << 8));
        uint16_t height = (uint16_t)(p[2] | (p[3] << 8));
        p += 4;

        size_t pixels_size = (size_t)width * (size_t)height;
        vm->cart->sprites[i].width = width;
        vm->cart->sprites[i].height = height;
        vm->cart->sprites[i].pixels = (uint8_t *)malloc(pixels_size);
        if (!vm->cart->sprites[i].pixels)
            return;

        memcpy(vm->cart->sprites[i].pixels, p, pixels_size);
        p += pixels_size;
    }
}

EMSCRIPTEN_KEEPALIVE
void load_sfx_from_buffer(const uint8_t *buffer, int sfx_count)
{
    if (!vm)
        return;
    if (!vm->cart)
        vm->cart = (Cart *)calloc(1, sizeof(Cart));
    if (!vm->cart)
        return;

    free_cart_sfx(vm->cart);

    vm->cart->sfx_count = (uint16_t)sfx_count;
    if (sfx_count <= 0)
        return;

    vm->cart->sounds = (Sound *)calloc(sfx_count, sizeof(Sound));
    if (!vm->cart->sounds)
        return;

    const uint8_t *p = buffer;
    for (int i = 0; i < sfx_count; i++)
    {
        uint16_t speed = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
        vm->cart->sounds[i].speed = speed;
        vm->cart->sounds[i].length = NOTE_COUNT;

        for (int n = 0; n < NOTE_COUNT; n++)
        {
            uint8_t volume = p[0];
            uint16_t frequency = (uint16_t)(p[1] | (p[2] << 8));
            uint8_t waveform = p[3];
            p += 4;

            vm->cart->sounds[i].notes[n].volume = volume;
            vm->cart->sounds[i].notes[n].frequency = frequency;
            vm->cart->sounds[i].notes[n].waveform = (WaveType)waveform;
        }
    }
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "0");
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);

    screen = screen_init(0); // Default to black
    if (!screen)
    {
        printf("Failed to initialize screen\n");
        return -1;
    }

    init_audio();
    set_errorHandler(web_error_handler);

    init_scanner(source);
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);    

    vm = init_vm(comp, screen);

    js_load_sprites_to_memory();
    js_load_sfx_to_memory();

    last_time = SDL_GetTicks();
    start_time = clock();

    // emscripten_set_main_loop(main_loop, 0, 1);
    emscripten_set_main_loop(main_loop, TARGET_FPS, 1);

    SDL_DestroyWindow(screen->window);
    SDL_DestroyRenderer(screen->renderer);
    SDL_Quit();
    return 0;
}

#else // Native version below

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pi_compiler.h"
#include "pi_vm.h"
#include "screen.h"
#include "common.h"
#include "./builtin/pi_audio.h"
#include "pi_lex.h"
#include "pi_parser.h"

Screen *screen;

static char *load_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", path);
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, length, file);
    if (read != (size_t)length)
    {
        fprintf(stderr, "Failed to read file: %s\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[read] = '\0';

    fclose(file);
    return buffer;
}

static void wait_for_exit(Screen *screen)
{
    SDL_Event event;
    const char *message = "Done. CTRL+C or close.";

    screen->offset_x = 0;
    screen->offset_y = 0;
    draw_fillRect(screen, 0, SCREEN_HEIGHT - CHAR_HEIGHT - 1,
                  SCREEN_WIDTH - 1, CHAR_HEIGHT, screen->clear_color);
    screen_print(screen, message, 1, SCREEN_HEIGHT - CHAR_HEIGHT, COLOR_WHITE);
    screen_update(screen);

    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return;

        if (event.type != SDL_KEYDOWN)
            continue;

        if (event.key.repeat == 0 &&
            event.key.keysym.sym == SDLK_f &&
            (event.key.keysym.mod & KMOD_CTRL))
        {
            screen_toggleFullscreen(screen);
            continue;
        }

        if (event.key.keysym.sym == SDLK_c &&
            (event.key.keysym.mod & KMOD_CTRL))
            return;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <script.pi>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        error("SDL_Init failed: %s", SDL_GetError());

    screen = screen_init(0); // Default to black
    if (!screen)
    {
        fprintf(stderr, "Failed to initialize screen\n");
        return 1;
    }
    init_audio();

    char *source = load_file(argv[1]);
    if (!source)
    {
        screen_close(screen);
        SDL_Quit();
        return 1;
    }

    compiler_t *comp = init_compiler();
    init_scanner(source);
    token_t *tokens = scan();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);

    vm_t *vm = init_vm(comp, screen);
    vm->source_path = strdup(argv[1]);
    vm->running = true;
    clock_t start_time = clock();
    run(vm);
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
    printf("Execution Time: %.4f ms\n", time_taken);
    fflush(stdout);
    if (!vm->close_requested)
        wait_for_exit(screen);

    free_parser(parser);
    free(source);
    free_compiler(comp);
    screen_close(screen);
    free_vm(vm);
    SDL_Quit();

    return 0;
}

#endif
