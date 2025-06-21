#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

#include "pi_io.h"

#include "../common.h"
#include "../screen.h"

/**
 * @brief Prints its arguments to the console followed by a newline.
 *
 * Converts each argument to a string and concatenates them together with
 * spaces in between. The resulting string is then printed to the console.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return Nil
 */
Value pi_println(vm_t *vm, int argc, Value *argv)
{

    char result[1024] = "";
    int offset = 0;

    for (int i = 0; i < argc; i++)
    {
        const char *str;
        bool is_allocated = false;
        str = as_string(argv[i]);

        int len = strlen(str);
        if (offset + len >= 1024)
            error("[println] Output string is too long.");

        strcat(result, str);
        offset += len;

        if (is_allocated)
            free((void *)str); // Only free if allocated
    }

    SDL_Delay(0);
    printf("%s\n", result);
    return NEW_NIL();
}

/**
 * @brief Prints its arguments to the console without a trailing newline.
 *
 * Converts each argument to a string and concatenates them together with
 * spaces in between. The resulting string is then printed to the console.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments passed to the function.
 * @param argv The arguments provided to the function.
 * @return Nil
 */
Value pi_print(vm_t *vm, int argc, Value *argv)
{

    if (argc == 0)
        error("[print] expects at least one argument.");

    for (int i = 0; i < argc; i++)
        print_value(argv[i], false);

    return NEW_NIL();
}

/**
 * @brief Prints a formatted string to the console.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments.
 * @param argv Arguments: format string followed by values to format into the string.
 *
 * The format string uses the following notation:
 * - `{n}` where `n` is a single digit, inserts the value at index `n` (0-indexed)
 * - `\\n` inserts a newline
 *
 * @return Nil
 */
Value pi_printf(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[printf] expects a format string as the first argument.");

    const char *format = AS_CSTRING(argv[0]);
    char result[1024] = "";

    for (const char *p = format; *p; p++)
    {
        if (*p == '{' && isdigit(*(p + 1)) && *(p + 2) == '}')
        {
            int index = *(p + 1) - '0';
            if (index + 1 >= argc)
                error("[printf] argument index out of range.");

            strcat(result, as_string(argv[index + 1]));
            p += 2; // Skip over the "{n}" part
        }
        else if (*p == '\\' && *(p + 1) == 'n')
        {
            strcat(result, "\n");
            p++; // Skip over the '\n'
        }
        else
            strncat(result, p, 1);
    }

    printf("%s", result);
    return NEW_NIL();
}

/**
 * @brief Renders text at a specified position on the screen.
 *
 * This function displays a given string at the specified (x, y) coordinates
 * on the screen. If only a single string argument is provided, the text is
 * rendered at the current cursor position. Optionally, a color can be specified.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function, which include:
 *             - x (integer): The x-coordinate for text rendering.
 *             - y (integer): The y-coordinate for text rendering.
 *             - text (string): The text to render.
 *             - color (optional integer): The color to use for rendering.
 * @return Nil
 */
Value pi_text(vm_t *vm, int argc, Value *argv)
{
    int x, y;
    const char *text;
    int color = 0;

    if (argc >= 3)
    {
        // First two arguments are cursor position
        x = as_number(argv[0]);
        y = as_number(argv[1]);
        text = as_string(argv[2]);
    }
    else if (argc == 1 && IS_STRING(argv[0]))
    {
        // Single string argument, print at default cursor position
        x = vm->screen->cursor_x;
        y = vm->screen->cursor_y;
        text = as_string(argv[0]);
    }

    if (argc >= 4 && IS_NUM(argv[3]))
        color = AS_NUM(argv[3]);
    // else
    //     error("[text] expects either 3 arguments (x, y, string) or 1 string argument.");

    screen_print(vm->screen, text, x, y, color);
    return NEW_NIL();
}

/**
 * @brief Maps a key name to its corresponding SDL_Scancode.
 *
 * This function takes a key name as a string and returns its corresponding
 * SDL_Scancode. The function handles the following key names:
 *
 *   - KEY_A to KEY_Z (letter keys)
 *   - KEY_0 to KEY_9 (number keys)
 *   - SPACE
 *   - ENTER
 *   - ESC
 *   - UP, DOWN, LEFT, RIGHT
 *   - LSHIFT, RSHIFT
 *   - LCTRL, RCTRL
 *   - LALT, RALT
 *
 * If the key name is not recognized, the function returns SDL_SCANCODE_UNKNOWN.
 * The function is case-insensitive.
 *
 * @param keyname The key name to map.
 * @return The corresponding SDL_Scancode.
 */
static SDL_Scancode get_keyCode(const char *keyname)
{
    // Strip "KEY_" prefix if present
    const char *key = keyname;
    if (strncmp(keyname, "KEY_", 4) == 0)
    {
        key = keyname + 4;
    }

    // Handle letter keys (KEY_A to KEY_Z)
    if (strlen(key) == 1 && isalpha(key[0]))
    {
        char upper = toupper(key[0]);
        return SDL_SCANCODE_A + (upper - 'A');
    }

    // Handle number keys (KEY_0 to KEY_9)
    if (strlen(key) == 1 && isdigit(key[0]))
    {
        return SDL_SCANCODE_0 + (key[0] - '0');
    }

    // Handle special keys
    if (strcmp(key, "SPACE") == 0)
        return SDL_SCANCODE_SPACE;
    if (strcmp(key, "ENTER") == 0)
        return SDL_SCANCODE_RETURN;
    if (strcmp(key, "ESC") == 0)
        return SDL_SCANCODE_ESCAPE;
    if (strcmp(key, "UP") == 0)
        return SDL_SCANCODE_UP;
    if (strcmp(key, "DOWN") == 0)
        return SDL_SCANCODE_DOWN;
    if (strcmp(key, "LEFT") == 0)
        return SDL_SCANCODE_LEFT;
    if (strcmp(key, "RIGHT") == 0)
        return SDL_SCANCODE_RIGHT;
    if (strcmp(key, "LSHIFT") == 0)
        return SDL_SCANCODE_LSHIFT;
    if (strcmp(key, "RSHIFT") == 0)
        return SDL_SCANCODE_RSHIFT;
    if (strcmp(key, "LCTRL") == 0)
        return SDL_SCANCODE_LCTRL;
    if (strcmp(key, "RCTRL") == 0)
        return SDL_SCANCODE_RCTRL;
    if (strcmp(key, "LALT") == 0)
        return SDL_SCANCODE_LALT;
    if (strcmp(key, "RALT") == 0)
        return SDL_SCANCODE_RALT;

    return SDL_SCANCODE_UNKNOWN;
}

/**
 * @brief Returns true if the given key is currently pressed.
 *
 * This function can be used to detect key presses. The first argument is the
 * key name as a string, such as "SPACE" or "ENTER". The second argument is
 * optional and specifies whether the key press should be detected only once
 * (true) or continuously (false). If the second argument is not provided, the
 * default is to detect the key press continuously.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments passed to the function.
 * @param argv The arguments provided to the function, which include:
 *             - keyname (string): The key name to detect.
 *             - once (optional bool): Whether the key press should be detected
 *               only once (true) or continuously (false). If not provided,
 *               defaults to false.
 * @return True if the key is pressed, false otherwise.
 */
Value pi_key(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        error("[key] expects at least one argument (string or number)");

    bool once = false;

    SDL_Scancode scancode;

    if (argc >= 2)
        once = as_bool(argv[1]);

    if (IS_STRING(argv[0]))
    {
        const char *keyname = as_string(argv[0]);
        scancode = get_keyCode(keyname);
        if (scancode == SDL_SCANCODE_UNKNOWN)
            error("[key] Unknown key name: %s", keyname);
    }
    else if (IS_NUM(argv[0]))
        scancode = (SDL_Scancode)as_number(argv[0]);
    else
        error("[key] Argument must be string or number");

    SDL_PumpEvents();
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    bool pressed = (scancode < SDL_NUM_SCANCODES) ? keystate[scancode] : false;

    if (once)
    {
        // Detect key press only once
        static bool prev_pressed = false;
        if (pressed && !prev_pressed)
        {
            prev_pressed = true;
            return NEW_BOOL(true);
        }
        else if (!pressed)
        {
            prev_pressed = false;
        }
        return NEW_BOOL(false);
    }
    else
        // Detect key press continuously
        return NEW_BOOL(pressed);
}

/**
 * @brief Prompts the user for input and returns it as a string.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [prompt string]
 * @return The input line as a string.
 */
Value pi_input(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || !IS_STRING(argv[0]))
        error("[input] expects a single string argument as a prompt.");

    PiString *prompt = AS_STRING(argv[0]);
    printf("%s", prompt->chars);
    fflush(stdout);

    char buffer[BUFFER_SIZE];
    if (!fgets(buffer, BUFFER_SIZE, stdin))
        error("[input] Failed to read input.");

    // Remove trailing newline if exists
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    return NEW_OBJ(new_pistring(strdup(buffer)));
}

/**
 * @brief Opens a file and returns a file handler.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1 or 2).
 * @param argv Arguments: [file path], [file mode]
 * @return A file handler object.
 */
Value pi_open(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_STRING(argv[0]))
        error("[open] expects a single string argument as a file path.");

    char *mode = "r";
    if (argc >= 2)
    {
        if (IS_STRING(argv[1]))
            mode = AS_CSTRING(argv[1]);
        else
            error("[open] expects a string argument as a file mode.");
    }

    PiString *path = AS_STRING(argv[0]);
    FILE *file = fopen(path->chars, mode);

    if (!file)
        error("[open] Failed to open file: %s", path->chars);

    // Extract filename from path
    const char *fullpath = path->chars;
    const char *last = strrchr(fullpath, '/');
#ifdef _WIN32
    // Windows uses backslashes as path separators
    const char *_last = strrchr(fullpath, '\\');
    if (!last || (_last && _last > last))
        last = _last;
#endif
    const char *filename = last ? last + 1 : fullpath;

    // Make a copy of filename and mode, since they must be owned by ObjFile
    char *_filename = strdup(filename);
    char *_mode = strdup(mode);
    ObjFile *f = (ObjFile *)new_file(file, _filename, _mode);

    return NEW_OBJ(f);
}

/**
 * @brief Reads a string from the file at the current position.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Argument: [file handler]
 * @return true if successful, otherwise raises an error.
 */

Value pi_read(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 || OBJ_TYPE(argv[0]) != OBJ_FILE)
        error("[read] expects a single file handler as argument.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        error("[read] File is closed.");

    size_t buffer_size = BUFFER_SIZE;
    size_t capacity = buffer_size;
    size_t length = 0;

    char *content = malloc(capacity);
    if (!content)
        error("[read] Out of memory.");

    while (!feof(file->fp))
    {
        if (length + buffer_size > capacity)
        {
            capacity *= 2;
            char *new_content = realloc(content, capacity);
            if (!new_content)
            {
                free(content);
                error("[read] Out of memory during read.");
            }
            content = new_content;
        }

        size_t bytes = fread(content + length, 1, buffer_size, file->fp);
        if (ferror(file->fp))
        {
            free(content);
            error("[read] Failed to read file: %s", file->filename);
        }
        length += bytes;
    }

    content[length] = '\0';

    Value result = NEW_OBJ(new_pistring(strdup(content)));
    free(content); // assuming new_pistring makes a copy
    return result;
}

/**
 * @brief Writes a string to the file at the current position.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 2).
 * @param argv Arguments: [file handler, string to write]
 * @return true if successful, otherwise raises an error.
 */
Value pi_write(vm_t *vm, int argc, Value *argv)
{

    if (argc != 2 || OBJ_TYPE(argv[0]) != OBJ_FILE)
        error("[write] expects a file handler and a string as arguments.");

    if (!IS_STRING(argv[1]))
        error("[write] second argument must be a string.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        error("[write] File is closed.");

    char *str = AS_CSTRING(argv[1]);

    size_t written = fwrite(str, 1, strlen(str), file->fp);

    if (written < strlen(str) || ferror(file->fp))
        error("[write] Failed to write to file: %s", file->filename);

    return NEW_BOOL(true); // or return number of bytes written if you want
}

/**
 * @brief Sets the file position to the given number of bytes from the beginning of the file.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 2).
 * @param argv Arguments: [file handler, byte position as number]
 * @return true if successful, otherwise raises an error.
 */
Value pi_seek(vm_t *vm, int argc, Value *argv)
{

    if (argc != 2 || OBJ_TYPE(argv[0]) != OBJ_FILE)
        error("[seek] expects a file handler and a number as arguments.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        error("[seek] File is closed.");

    if (!IS_NUM(argv[1]))
        error("[seek] second argument must be a number.");

    long pos = as_number(argv[1]);
    if (fseek(file->fp, pos, SEEK_SET) != 0)
        error("[seek] Failed to seek in file: %s", file->filename);

    return NEW_BOOL(true);
}

/**
 * @brief Closes the file stream and marks it as closed.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Argument: [file handler]
 * @return true if successful, otherwise raises an error.
 */
Value pi_close(vm_t *vm, int argc, Value *argv)
{

    if (argc != 1 || OBJ_TYPE(argv[0]) != OBJ_FILE)
        error("[close] expects a file handler as argument.");

    ObjFile *file = AS_FILE(argv[0]);

    if (fclose(file->fp) != 0)
        error("[close] Failed to close file: %s", file->filename);

    file->closed = true;
    return NEW_BOOL(true);
}
