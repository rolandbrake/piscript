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

// Native (non-Emscripten) includes and main
#include <SDL2/SDL.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "pi_compiler.h"
#include "pi_vm.h"
#include "pi_shell.h"
#include "screen.h"
#include "common.h"
#include "commands.h"
#include "./builtin/pi_audio.h"
// New includes
#include "pi_lex.h"    // For scanner
#include "pi_parser.h" // For parser

#define CHAR_WIDTH 4
#define CHAR_HEIGHT 6

// --- Forward declarations for shell implementation ---

static void shell_write(const char *s, ...);
static void shell_read(char *buffer, int size, ...);
static void shell_clear(Color color);
static void scroll_screen(Screen *screen);
static void vmshell_run(vm_t *vm);

// --- VM Shell I/O implementations ---

static void shell_write(const char *s, ...)
{
    if (!shell_io || !shell_io->vm)
        return;
    Screen *screen = shell_io->vm->screen;

    int color = COLOR_WHITE; // Default color

    va_list args;
    va_start(args, s);

    shell_opt opt = va_arg(args, shell_opt);
    while (opt != SHELL_END)
    {
        switch (opt)
        {
        case SHELL_COLOR:
            color = va_arg(args, int);
            break;

        case SHELL_CURSOR_X:
            screen->cursor_x = va_arg(args, int);
            break;

        case SHELL_CURSOR_Y:
            screen->cursor_y = va_arg(args, int);
            break;

        default:
            // Ignore unknown options
            break;
        }
        opt = va_arg(args, shell_opt);
    }

    va_end(args);

    for (const char *p = s; *p; p++)
    {
        if (*p == '\n')
        {
            screen->cursor_x = 1;
            screen->cursor_y += CHAR_HEIGHT;
        }
        else
        {
            char _char[2] = {*p, '\0'};
            screen_print(screen, _char, screen->cursor_x, screen->cursor_y, color);
        }

        if (screen->cursor_y > SCREEN_HEIGHT - CHAR_HEIGHT)
        {
            scroll_screen(screen);
            screen->cursor_y = SCREEN_HEIGHT - CHAR_HEIGHT;
        }
    }
    screen_update(screen);
}

static void shell_read(char *buffer, int size, ...)
{
    if (!shell_io || !shell_io->vm)
    {
        buffer[0] = '\0';
        shell_stop();
        return;
    }

    Screen *screen = shell_io->vm->screen;

    bool is_repl = false;

    int text_color = COLOR_WHITE;
    const int bg_color = COLOR_BLACK;

    va_list args;
    va_start(args, size);

    shell_opt opt = va_arg(args, shell_opt);
    while (opt != SHELL_END)
    {
        switch (opt)
        {
        case SHELL_COLOR:
            text_color = va_arg(args, int);
            break;
        case REPL_MODE:
            is_repl = va_arg(args, int);
            break;

        default:
            // Ignore unknown options
            break;
        }
        opt = va_arg(args, shell_opt);
    }

    va_end(args);

    const char *prompt = is_repl ? ">>" : ">";
    int prompt_color = COLOR_DARK_GRAY;
    // int cursor_color = is_repl ? COLOR_BRIGHT_RED : COLOR_LIME_GREEN;
    int cursor_color = COLOR_BRIGHT_RED;

    const int cols_per_line =
        (SCREEN_WIDTH / CHAR_WIDTH) - strlen(prompt) - 1;

    int len = 0;
    int cursor_pos = 0;
    buffer[0] = '\0';

    int base_y = screen->cursor_y;

    SDL_StartTextInput();
    bool reading = true;

    static bool wave_initialized = false;
    static Uint32 last_wave_start = 0;
    static Uint32 wave_start = 0;
    static Uint32 last_wave_frame = 0;
    static bool wave_active = false;

    while (reading)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                reading = false;
                shell_stop();
                shell_io->vm->running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_f && (event.key.keysym.mod & KMOD_CTRL))
                {
                    Uint32 flags = SDL_GetWindowFlags(screen->window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        SDL_SetWindowFullscreen(screen->window, 0);
                    else
                        SDL_SetWindowFullscreen(screen->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {
                    if (history_pos > 0)
                    {
                        history_pos--;
                        strcpy(buffer, history[history_pos % HISTORY_MAX]);
                        len = strlen(buffer);
                        cursor_pos = len;
                    }
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    if (history_pos < history_count - 1)
                    {
                        history_pos++;
                        strcpy(buffer, history[history_pos % HISTORY_MAX]);
                        len = strlen(buffer);
                        cursor_pos = len;
                    }
                    else
                    {
                        history_pos = history_count;
                        buffer[0] = '\0';
                        len = 0;
                        cursor_pos = 0;
                    }
                }
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    if (cursor_pos > 0)
                        cursor_pos--;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    if (cursor_pos < len)
                        cursor_pos++;
                }
                else if (event.key.keysym.sym == SDLK_RETURN ||
                         event.key.keysym.sym == SDLK_KP_ENTER)
                {
                    reading = false;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && cursor_pos > 0)
                {
                    memmove(&buffer[cursor_pos - 1], &buffer[cursor_pos], len - cursor_pos + 1);
                    len--;
                    cursor_pos--;
                }
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                int add = strlen(event.text.text);
                if (len + add < size - 1)
                {
                    memmove(&buffer[cursor_pos + add], &buffer[cursor_pos], len - cursor_pos + 1);
                    memcpy(&buffer[cursor_pos], event.text.text, add);
                    len += add;
                    cursor_pos += add;
                }
            }
        }

        if (!is_repl && shell_home_visible)
        {
            Uint32 now = SDL_GetTicks();
            if (!wave_initialized)
            {
                last_wave_start = now;
                wave_initialized = true;
            }
            if (!wave_active && (now - last_wave_start) >= 20000)
            {
                wave_active = true;
                wave_start = now;
                last_wave_frame = 0;
            }

            if (wave_active)
            {
                int clear_y = LOGO_Y - 2;
                int clear_h = LOGO_H + 4;
                if ((now - wave_start) >= 2000)
                {
                    wave_active = false;
                    last_wave_start = now;
                    draw_fillRect(screen, LOGO_X, clear_y, LOGO_W, clear_h, COLOR_BLACK);
                    draw_logo(LOGO_X, LOGO_Y);
                    screen_update(screen);
                }
                else if (now - last_wave_frame >= 60)
                {
                    last_wave_frame = now;
                    draw_fillRect(screen, LOGO_X, clear_y, LOGO_W, clear_h, COLOR_BLACK);
                    draw_logoWave(LOGO_X, LOGO_Y, (now - wave_start) * 0.02);
                    screen_update(screen);
                }
            }
        }

        /* compute wrapping */
        int rows = (len / cols_per_line) + 1;

        /* scroll if needed */
        while (base_y + rows * CHAR_HEIGHT > SCREEN_HEIGHT)
        {
            scroll_screen(screen);
            base_y -= CHAR_HEIGHT;
            screen->cursor_y = base_y;
        }

        /* clear only needed rows */
        for (int r = 0; r <= rows; r++)
        {
            draw_fillRect(
                screen,
                0,
                base_y + r * CHAR_HEIGHT,
                SCREEN_WIDTH,
                CHAR_HEIGHT,
                bg_color);
        }

        /* prompt on first row */
        screen_print(screen, prompt, 1, base_y, prompt_color);

        bool is_string = false;

        /* print input across lines */
        if (is_repl)
        {
            for (int i = 0; i < len; i++)
            {
                int r = i / cols_per_line;
                int c = i % cols_per_line;

                int x = (r == 0 ? strlen(prompt) + c : c) * CHAR_WIDTH + 1;

                int y = base_y + r * CHAR_HEIGHT;

                char ch[2] = {buffer[i], '\0'};
                Color color = text_color;

                switch (buffer[i])
                {
                case '(':
                case ')':
                    color = COLOR_BRIGHT_BLUE;
                    break;
                case '[':
                case ']':
                    color = COLOR_BRIGHT_GREEN;
                    break;
                case '{':
                case '}':
                    color = COLOR_VERY_LIGHT_PINK;
                    break;
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                case '<':
                case '>':
                case '!':
                    color = COLOR_BRIGHT_RED;
                    break;
                case '"':
                case '\'':
                {
                    is_string = !is_string;
                    color = COLOR_BRIGHT_BLUE;
                    break;
                }
                default:
                    if (isdigit(buffer[i]))
                        color = COLOR_BRIGHT_GREEN;
                    if (is_string)
                        color = COLOR_BRIGHT_BLUE;
                    else if (isalpha(buffer[i]) || buffer[i] == '_')
                    {
                        if (strcmp(buffer, "exit") == 0 ||
                            strcmp(buffer, "quit") == 0 ||
                            strcmp(buffer, "cls") == 0)
                            color = COLOR_WHITE;
                        else if (strcmp(buffer, "nil") == 0 ||
                                 strcmp(buffer, "true") == 0 ||
                                 strcmp(buffer, "false") == 0)
                            color = COLOR_BRIGHT_RED;
                        else
                            color = COLOR_BRIGHT_PINK;
                    }
                    break;
                }

                screen_print(screen, ch, x, y, color);
            }
        }
        else // This is the non-REPL mode, where shell commands are entered
        {
            // Identify the command word (first token)
            char command_name[LINE_MAX];
            command_name[0] = '\0';
            sscanf(buffer, "%s", command_name);

            Color cmd_color = COLOR_WHITE; // Default color for text
            bool is_command = false;

            for (int i = 0; i < num_commands(); i++)
            {
                if (strcmp(command_name, commands[i].name) == 0)
                {
                    cmd_color = COLOR_WARM_GRAY; // Color for recognized commands
                    is_command = true;
                    break;
                }
            }

            for (int i = 0; i < len; i++)
            {
                int r = i / cols_per_line;
                int c = i % cols_per_line;

                int x =
                    (r == 0 ? strlen(prompt) + c : c) * CHAR_WIDTH + 1;

                int y = base_y + r * CHAR_HEIGHT;

                char ch[2] = {buffer[i], '\0'};
                Color current_charColor = text_color;

                if (is_command && i < strlen(command_name))
                {
                    current_charColor = cmd_color;
                }
                else if (isspace(buffer[i]) && is_command)
                {
                    is_command = false; // Stop coloring command once space is encountered
                }

                screen_print(screen, ch, x, y, current_charColor);
            }
        }

        /* blinking cursor */
        if ((SDL_GetTicks() / 500) & 1 && reading &&
            (SDL_GetWindowFlags(screen->window) & SDL_WINDOW_INPUT_FOCUS))
        {
            int cursor_row = cursor_pos / cols_per_line;
            int cursor_col = cursor_pos % cols_per_line;
            int cx =
                (cursor_row == 0 ? strlen(prompt) + cursor_col : cursor_col) * CHAR_WIDTH + 1;

            int cy = base_y + cursor_row * CHAR_HEIGHT;

            draw_fillRect(
                screen,
                cx,
                cy,
                CHAR_WIDTH,
                CHAR_HEIGHT,
                cursor_color);
        }

        screen_update(screen);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    shell_write("\n", SHELL_END);

    if (len > 0)
    {
        /* avoid duplicate consecutive entries */
        if (history_count == 0 ||
            strcmp(history[(history_count - 1) % HISTORY_MAX], buffer) != 0)
        {
            strncpy(history[history_count % HISTORY_MAX], buffer, LINE_MAX - 1);
            history[history_count % HISTORY_MAX][LINE_MAX - 1] = '\0';
            history_count++;
        }
    }

    /* reset navigation */
    history_pos = history_count;
}

static void shell_clear(Color color)
{
    if (!shell_io || !shell_io->vm)
        return;

    Screen *screen = shell_io->vm->screen;
    screen_clear(screen, color);
    screen->cursor_x = 1;
    screen->cursor_y = 1;
    screen_update(screen);
}

// Helper to scroll the screen up by one line
static void scroll_screen(Screen *screen)
{
    // Shift pixel buffer up by CHAR_HEIGHT lines
    memmove(screen->pixels,
            screen->pixels + (SCREEN_WIDTH * CHAR_HEIGHT),
            SCREEN_WIDTH * (SCREEN_HEIGHT - CHAR_HEIGHT) * sizeof(Uint32));

    // Clear the last line
    for (int y = SCREEN_HEIGHT - CHAR_HEIGHT; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
            screen->pixels[y * SCREEN_WIDTH + x] = 0; // Black
    }
}

Screen *screen;

static char *load_file(const char *path)
{
    FILE *file = fopen(path, "r");
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

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[])
{
    printf("PiScript v0.0.1\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        error("SDL_Init failed: %s", SDL_GetError());

    screen = screen_init(0); // Default to black
    if (!screen)
    {
        fprintf(stderr, "Failed to initialize screen\n");
        return 1;
    }
    init_audio();

    // Create a compiler and vm to pass to the shell
    compiler_t *comp = init_compiler();
    vm_t *vm = init_vm(comp, screen);

    // --- Start of new code for boot.pi ---
    char *source = load_file("boot.pi");
    if (!source)
    {
        fprintf(stderr, "Failed to load boot.pi\n");
        return 1;
    }

    init_scanner(source);
    token_t *tokens = scan();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);

    vm->running = true;
    run(vm);             // Execute boot.pi
    vm->running = false; // Stop the VM after boot.pi

    // Clear screen and reset VM for the shell
    screen_clear(screen, COLOR_BLACK);
    vm_reset(vm, comp); // Reset VM to clear state from boot.pi execution
    // --- End of new code for boot.pi ---

    screen_clear(screen, COLOR_BLACK);
    // The shell is the main program now.
    // It will loop until the user quits.
    shell_io_t shell_io = {
        .out = shell_write,
        .in = shell_read,
        .clear = shell_clear,
        .vm = vm};

    shell_run(&shell_io);

    free_compiler(comp);

    return 0;
}

#endif
