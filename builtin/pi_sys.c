#include <math.h>
#include "pi_sys.h"
#include "../pi_value.h"
#include "../list.h"
#include "pi_plot.h"

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

    return NEW_OBJ(new_pistring(type));
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
        vm->screen->text_color = (Color)AS_NUM(argv[2]);

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

Value pi_zen(vm_t *vm, int argc, Value *argv)
{

    return NEW_OBJ(new_pistring(

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
        "----------------------------------------\n"));
}
