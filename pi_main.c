#ifdef __EMSCRIPTEN__

// Emscripten-specific includes and code

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>
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

    // Uint32 frame_time = SDL_GetTicks() - frame_start;
    // if (frame_time < 1000 / TARGET_FPS)
    //     SDL_Delay((1000 / TARGET_FPS) - frame_time);

    emscripten_sleep(0);
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

    emscripten_cancel_main_loop();

    // screen_clear(screen, 12);
    // screen_update(screen);
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

extern Sprite *sprites; // make sure this is declared properly

EM_JS(void, js_load_sprites_to_memory, (), {
    const saved = localStorage.getItem('savedSprites');
    if (!saved)
        return;

    const sprites = JSON.parse(saved);
    const spriteCount = sprites.length;

    const buffer = Module._malloc(256 * 2 * 2 + 256 * 256); // Max metadata + max total pixel buffer

    let offset = buffer;
    for (let i = 0; i < spriteCount; i++)
    {
        const s = sprites[i];
        const width = s.width;
        const height = s.height;
        const pixels = s.pixels;

        Module.HEAPU16[offset >> 1] = width;
        Module.HEAPU16[(offset >> 1) + 1] = height;
        offset += 4;

        for (let j = 0; j < pixels.length; j++)
        {
            Module.HEAPU8[offset + j] = pixels[j];
        }
        offset += width * height;
    }

    Module.ccall('load_sprites_from_buffer', // name of C function
                 null,                       // return type
                 [ 'number', 'number' ],     // args
                 [ buffer, spriteCount ]);
});

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

    screen = screen_init();
    if (!screen)
    {
        printf("Failed to initialize screen\n");
        return -1;
    }

    init_audio();

    init_scanner(source);
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens);
    parse(parser);

    vm = init_vm(comp, screen);
    last_time = SDL_GetTicks();
    start_time = clock();

    emscripten_set_main_loop(main_loop, 0, 1);
    SDL_DestroyWindow(screen->window);
    SDL_DestroyRenderer(screen->renderer);
    SDL_Quit();
    return 0;
}

#else // Native version below

// Native (non-Emscripten) includes and main
#include <SDL2/SDL.h>
#include <pthread.h>
#include <time.h>
#include "pi_lex.h"
#include "pi_parser.h"
#include "pi_stack.h"
#include "pi_compiler.h"
#include "pi_vm.h"
#include "screen.h"
#include "common.h"
#include "./builtin/pi_audio.h"

#ifndef TARGET_FPS
#define TARGET_FPS 60
#endif

Screen *screen;

static void *vm_run(void *arg)
{
    vm_t *vm = (vm_t *)arg;
    clock_t start_time = clock();
    run(vm);
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
    printf("Execution Time: %.4f ms\n", time_taken);
    return NULL;
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

void load_sprite_file(const char *filename)
{
    Sprite *sprites = screen->sprites;

    FILE *f = fopen(filename, "rb");
    if (!f)
        return;

    fread(&screen->sprite_count, sizeof(uint8_t), 1, f);

    for (int i = 0; i < screen->sprite_count; i++)
    {
        fread(&sprites[i].width, sizeof(uint8_t), 1, f);
        fread(&sprites[i].height, sizeof(uint8_t), 1, f);

        int size = sprites[i].width * sprites[i].height;
        sprites[i].pixels = malloc(size);
        fread(sprites[i].pixels, sizeof(uint8_t), size, f);
    }

    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "No command provided.\nUse 'pi help' for available commands.\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "help") == 0)
    {
        printf("Usage:\n");
        printf("  pi run <file>     Run the specified Piscript file\n");
        printf("  pi run            Launch the Piscript REPL (not yet implemented)\n");
        printf("  pi min <file>     Minimize the specified Piscript file (coming soon)\n");
        printf("  pi fmt <file>     Format the specified Piscript file (coming soon)\n");
        printf("  pi help           Display this help message\n");
        return 0;
    }

    if (strcmp(command, "run") == 0)
    {
        if (argc == 3)
        {
            const char *filename = argv[2];

            init_audio();
            screen = screen_init();

            char *source = read_file(filename);
            load_sprite_file("test.dat"); // You can customize sprite loading per file later
            init_scanner(source);
            token_t *tokens = scan();

#ifdef DEBUG_BUILD
            token_t *token = tokens;
            int i = 0;
            while (token->type != TK_EOF)
            {
                printf("%d: %s\n", i++, token_toString(*token));
                token++;
            }
            printf("%d: %s\n", i, token_toString(*token));
#endif

            compiler_t *comp = init_compiler();
            parser_t *parser = init_parser(comp, tokens);
            parse(parser);
            dis(comp);

            vm_t *vm = init_vm(comp, screen);

            pthread_t vm_thread;
            pthread_create(&vm_thread, NULL, vm_run, vm);

            bool running = true;
            SDL_Event *event = malloc(sizeof(SDL_Event));
            int frame_count = 0;
            double fps = 0.0;
            Uint32 last_time = SDL_GetTicks();

            while (running)
            {
                Uint32 frame_start = SDL_GetTicks();

                if (SDL_WaitEventTimeout(event, 1000 / TARGET_FPS))
                {
                    if (event->type == SDL_QUIT ||
                        (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE))
                    {
                        running = false;
                        pthread_mutex_lock(&vm->lock);
                        vm->running = false;
                        pthread_mutex_unlock(&vm->lock);
                        SDL_Delay(1000 / TARGET_FPS);
                    }
                }

                frame_count++;
                Uint32 current_time = SDL_GetTicks();
                if (current_time - last_time >= 1000)
                {
                    fps = frame_count / ((current_time - last_time) / 1000.0);
                    frame_count = 0;
                    last_time = current_time;
                    vm->fps = fps;
                }

                Uint32 frame_time = SDL_GetTicks() - frame_start;
                if (frame_time < 1000 / TARGET_FPS)
                    SDL_Delay((1000 / TARGET_FPS) - frame_time);
            }

            printf("Shutting down...\n");
            screen_close(screen);
            pthread_join(vm_thread, NULL);

            free(source);
            free_parser(parser);
            free_vm(vm);
            return 0;
        }
        else if (argc == 2)
        {
            printf("REPL is not yet implemented.\n");
            return 0;
        }
        else
        {
            fprintf(stderr, "Usage: pi run <file>\n");
            return 1;
        }
    }

    if (strcmp(command, "min") == 0 || strcmp(command, "fmt") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: pi %s <file>\n", command);
            return 1;
        }
        printf("Command '%s' on file '%s' is not yet implemented.\n", command, argv[2]);
        return 0;
    }

    fprintf(stderr, "Unknown command: %s\nUse 'pi help' to see available commands.\n", command);
    return 1;
}

#endif
