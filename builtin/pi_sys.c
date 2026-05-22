#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pi_sys.h"
#include "../pi_value.h"
#include "../pi_list.h"
#include "../pi_lex.h"
#include "../pi_parser.h"
#include "../pi_compiler.h"
#include "pi_plot.h"

typedef struct
{
    int pc;
    int sp;
    int bp;
    int ip;
    int iter_sp;
    int frame_sp;
    int counter;
    int next_gc;
    bool running;
    Uint32 frame_interval;
    Uint32 last_draw_ticks;
    list_t *code;
    list_t *constants;
    list_t *names;
    table_t *instrs;
    Object *function;
    UpValue *open_upvalues;
    Value stack[STACK_MAX];
    Frame *frames[STACK_MAX];
    Object *iters[STACK_MAX];
} nested_vm_t;

static void save_vm(vm_t *vm, nested_vm_t *state)
{
    state->pc = vm->pc;
    state->sp = vm->sp;
    state->bp = vm->bp;
    state->ip = vm->ip;
    state->iter_sp = vm->iter_sp;
    state->frame_sp = vm->frame_sp;
    state->counter = vm->counter;
    state->next_gc = vm->next_gc;
    state->running = vm->running;
    state->frame_interval = vm->frameInterval_ms;
    state->last_draw_ticks = vm->last_drawTicks;
    state->code = vm->code;
    state->constants = vm->constants;
    state->names = vm->names;
    state->instrs = vm->instrs;
    state->function = vm->function;
    state->open_upvalues = vm->openUpvalues;

    memcpy(state->stack, vm->stack, sizeof(state->stack));
    memcpy(state->frames, vm->frames, sizeof(state->frames));
    memcpy(state->iters, vm->iters, sizeof(state->iters));
}

static void restore_vm(vm_t *vm, const nested_vm_t *state)
{
    vm->pc = state->pc;
    vm->sp = state->sp;
    vm->bp = state->bp;
    vm->ip = state->ip;
    vm->iter_sp = state->iter_sp;
    vm->frame_sp = state->frame_sp;
    vm->counter = state->counter;
    vm->next_gc = state->next_gc;
    vm->running = state->running;
    vm->frameInterval_ms = state->frame_interval;
    vm->last_drawTicks = state->last_draw_ticks;
    vm->code = state->code;
    vm->constants = state->constants;
    vm->names = state->names;
    vm->instrs = state->instrs;
    vm->function = state->function;
    vm->openUpvalues = state->open_upvalues;

    memcpy(vm->stack, state->stack, sizeof(state->stack));
    memcpy(vm->frames, state->frames, sizeof(state->frames));
    memcpy(vm->iters, state->iters, sizeof(state->iters));
}

static Value eval_expr(vm_t *vm, char *expr)
{
    nested_vm_t caller;
    save_vm(vm, &caller);

    init_scanner(expr);
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens, MODE_REPL);
    parse(parser);

    vm_reset(vm, comp);
    run(vm);

    Value result = NEW_NIL();
    if (vm->sp > vm->bp)
        result = vm->stack[vm->sp - 1];

    restore_vm(vm, &caller);
    free_parser(parser);
    free_compiler(comp);
    return result;
}

static void run_script(vm_t *vm, char *source, const char *path)
{
    nested_vm_t caller;
    save_vm(vm, &caller);

    init_scanner(source);
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);

    char *caller_path = vm->source_path;
    vm->source_path = strdup(path);

    vm_reset(vm, comp);
    run(vm);

    free(vm->source_path);
    vm->source_path = caller_path;
    restore_vm(vm, &caller);
    free_parser(parser);
    free_compiler(comp);
}

static char *resolve_runPath(vm_t *vm, const char *path)
{
    char *resolved = resolve_sourcePath(vm->source_path, path);
    if (!resolved)
        vm_error(vm, "[pi_run] not enough memory to resolve path.");

    return resolved;
}

static char *read_source(vm_t *vm, const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        vm_errorf(vm, "[pi_run] could not open file: %s", path);

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        vm_errorf(vm, "[pi_run] could not read file: %s", path);
    }

    long length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        vm_errorf(vm, "[pi_run] could not read file: %s", path);
    }

    char *source = malloc((size_t)length + 1);
    if (!source)
    {
        fclose(file);
        vm_error(vm, "[pi_run] not enough memory to read source.");
    }

    size_t read = fread(source, 1, (size_t)length, file);
    fclose(file);
    if (read != (size_t)length)
    {
        free(source);
        vm_errorf(vm, "[pi_run] could not read file: %s", path);
    }

    source[length] = '\0';
    return source;
}

Value pi_fps(vm_t *vm, int argc, Value *argv)
{
    int fps = round(vm->fps);
    return NEW_NUM(fps);
}

Value _pi_type(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        vm_error(vm, "[type] expects at least one argument.");

    char *type = type_name(argv[0]);

    return NEW_OBJ(new_pistring(strdup(type)));
}

Value pi_error(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
        vm_error(vm, "[error] expects at least one argument.");

    const char *str = as_string(argv[0]);
    printf("Error: %s\n", str);
    free((void *)str);
    return NEW_NIL();
}

/**
 * @brief Sets the cursor position on the screen.
 *
 * This function takes two or three numeric arguments: the x and y coordinates, and
 * optionally the text color index. The text color index is wrapped within 32. An
 * error is raised if less than two arguments are provided.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (2 or 3).
 * @param argv The arguments: x, y, and optionally text_color.
 * @return A nil value indicating completion.
 */
Value pi_cursor(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        vm_error(vm, "[cursor] expects at least x and y.");

    int x = (int)as_number(argv[0]);
    int y = (int)as_number(argv[1]);

    vm->screen->cursor_x = x;
    vm->screen->cursor_y = y;

    if (argc >= 3 && IS_NUM(argv[2]))
    {
        Uint32 text_color = 0;
        if (!screen_colorFromNumber(AS_NUM(argv[2]), &text_color))
            vm_error(vm, "[cursor] text color must be a palette index or packed 0xAARRGGBB number.");
        vm->screen->text_color = text_color;
    }

    return NEW_NIL();
}

/**
 * Returns the current mouse position relative to the 128x128 virtual screen.
 *
 * This function takes no arguments and returns a list containing the x and y
 * coordinates of the mouse.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (should be 0).
 * @param argv The arguments (empty list).
 * @return A list containing the x and y coordinates of the mouse.
 */
Value pi_mouse(vm_t *vm, int argc, Value *argv)
{
    int x, y;
    // This already returns the mouse position *relative to the window*
    Uint32 buttons = SDL_GetMouseState(&x, &y);

    // Scale down to match your 128x128 virtual screen
    x /= SCALE;
    y /= SCALE;

    // Clamp to 0..127 to ensure it's within bounds
    if (x < 0)
        x = 0;
    if (x > 127)
        x = 127;
    if (y < 0)
        y = 0;
    if (y > 127)
        y = 127;

    list_t *list = list_create(sizeof(Value));
    list_add(list, &NEW_NUM(x));
    list_add(list, &NEW_NUM(y));

    return NEW_OBJ(new_list(list));
}

Value pi_eval(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[pi_eval] expects one string expression.");

    return eval_expr(vm, AS_CSTRING(argv[0]));
}

Value pi_run(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[pi_run] expects one file path string.");

    const char *path = AS_CSTRING(argv[0]);
    char *resolved = resolve_runPath(vm, path);
    char *source = read_source(vm, resolved);
    run_script(vm, source, resolved);
    free(source);
    free(resolved);
    return NEW_NIL();
}

Value pi_dis(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[pi_dis] expects one code string.");

    init_scanner(AS_CSTRING(argv[0]));
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);

    dis(comp);

    free_parser(parser);
    free_compiler(comp);
    return NEW_NIL();
}

Value pi_zen(vm_t *vm, int argc, Value *argv)
{

    return NEW_OBJ(new_pistring(strdup(

        "*********************************************\n"
        " ____ ___ ____   ____ ____  ___ ____ _____  \n"
        "|  _ \\_ _/ ___| / ___|  _ \\|_ _|  _ \\_   _|\n"
        "| |_) | |\\___ \\| |   | |_) || || |_) || |  \n"
        "|  __/| | ___) | |___|  _ < | ||  __/ | |  \n"
        "|_|  |___|____/ \\____|_| \\_\\___|_|    |_|  \n"
        "*********************************************\n"

        "\n"
        " The Zen of PiScript\n"
        " --------------------\n"
        " 1. Simplicity is power.\n"
        " 2. Functions shape the flow.\n"
        " 3. Tables hold the world.\n"
        " 4. Graphics tell the story.\n"
        " 5. 128 by 128, a universe unfolds.\n"
        " 6. Freedom in code, structure in choice.\n"
        " 7. Dynamic, yet precise.\n"
        " 8. Expressive, yet concise.\n"
        " 9. Less syntax, more meaning.\n"
        "10. A script should feel like art.\n"
        "\n"
        "PiScript is a canvas—paint with logic.\n"
        "----------------------------------------\n")));
}
