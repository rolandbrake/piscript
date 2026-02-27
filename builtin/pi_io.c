#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "pi_io.h"

#include "../common.h"
#include "../screen.h"

/**
 * @brief Appends a string to the given buffer.
 *
 * This function appends a string to the given buffer. It takes care to not
 * overflow the buffer. If the buffer is full, it does nothing.
 *
 * @param buffer The buffer to append to.
 * @param offset A pointer to the offset of the buffer.
 * @param text The string to append.
 */
static void append(char *buffer, int *offset, const char *text)
{
    int remaining = BUFFER_SIZE - *offset - 1;
    if (remaining <= 0)
        return;

    // Calculate how much of the string can be written into the buffer
    int written = snprintf(buffer + *offset, remaining, "%s", text);
    if (written > 0)
        // Update the offset to reflect the amount of characters written
        *offset += written;
}

/**
 * @brief Appends a single character to the given buffer.
 *
 * This function appends a single character to the given buffer. It takes
 * care to not overflow the buffer. If the buffer is full, it does
 * nothing.
 *
 * @param buffer The buffer to append to.
 * @param offset A pointer to the offset of the buffer.
 * @param c The character to append.
 */
static void append_char(char *buffer, int *offset, char c)
{
    // Check if the buffer is full
    if (*offset >= BUFFER_SIZE - 1)
        return;

    // Append the character
    buffer[*offset] = c;
    (*offset)++;

    // Null terminate the buffer
    buffer[*offset] = '\0';
}

/**
 * @brief Prints a string on the screen.
 *
 * This function takes one or three arguments: the text to be printed, and
 * optionally the x and y coordinates of the text position, and the text
 * color index. The text color index is wrapped within 32. An error is
 * raised if less than one argument is provided.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1 to 3).
 * @param argv The arguments: text (string), x (integer, optional), y (integer, optional), and text_color (integer, optional).
 * @return A nil value indicating completion.
 */
Value pi_print(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[print] expects at least text.");

    const char *text = as_string(argv[0]);

    int x = vm->screen->cursor_x;
    int y = vm->screen->cursor_y;
    int color = vm->screen->text_color;

    // Optional arguments
    if (argc >= 3)
    {
        x = (int)as_number(argv[1]);
        y = (int)as_number(argv[2]);
    }

    if (argc >= 4 && IS_NUM(argv[3]))
        color = (int)AS_NUM(argv[3]);

    screen_print(vm->screen, text, x, y, color);

    return NEW_NIL();
}

/**
 * @brief Prints a string on the screen followed by a newline character.
 *
 * This function takes one or four arguments: the text to be printed, and
 * optionally the x and y coordinates of the text position, and the text
 * color index. The text color index is wrapped within 32. An error is
 * raised if less than one argument is provided.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1 to 4).
 * @param argv The arguments: text (string), x (integer, optional), y (integer, optional), and text_color (integer, optional).
 * @return A nil value indicating completion.
 */
Value pi_println(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0)
    {
        // Just move to the next line if no text is provided
        vm->screen->cursor_x = 1;
        vm->screen->cursor_y += 6; // Move down by 6 pixels for
        return NEW_NIL();
    }

    // Get the text to be printed
    const char *text = as_string(argv[0]);

    // Get the x and y coordinates of the text position
    int x = vm->screen->cursor_x;
    int y = vm->screen->cursor_y;

    // Get the text color index
    int color = vm->screen->text_color;

    // Parse optional arguments
    if (argc >= 3)
    {
        // Get the x coordinate
        x = (int)as_number(argv[1]);

        // Get the y coordinate
        y = (int)as_number(argv[2]);
    }

    if (argc >= 4 && IS_NUM(argv[3]))
    {
        // Get the text color index
        color = (int)AS_NUM(argv[3]);
    }

    // Print the text
    screen_print(vm->screen, text, x, y, color);

    // Move the cursor to the next line
    vm->screen->cursor_x = 1;
    vm->screen->cursor_y = y + 6;

    return NEW_NIL();
}

/**
 * @brief Prints a formatted string on the screen.
 *
 * This function takes one or more arguments: the format string, and
 * optionally any number of values to be formatted into the string.
 * The format string is expected to contain placeholders in the form of
 * {index:color} where index is the 0-based index of the value to be
 * formatted, and color is the text color index to use for the formatted
 * value.
 *
 * The format string is also expected to contain newline characters (\n) which
 * will move the cursor to the next line.
 *
 * The function is case-insensitive.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1 to N).
 * @param argv The arguments: format (string), and optionally values to be formatted.
 * @return A nil value indicating completion.
 */
Value pi_printf(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[printf] expects format string.");

    const char *format = AS_CSTRING(argv[0]);

    char result[BUFFER_SIZE];
    result[0] = '\0';
    int offset = 0;

    for (const char *p = format; *p; p++)
    {
        // Handle {index:color}
        if (*p == '{')
        {
            const char *start = p;
            p++;

            if (!isdigit(*p))
            {
                p = start;
                append_char(result, &offset, *p);
                continue;
            }

            int index = *p - '0';
            p++;

            int color = vm->screen->text_color;

            if (*p == ':')
            {
                p++;
                if (!isdigit(*p))
                    vm_error(vm, "[printf] invalid color format.");

                color = *p - '0';
                p++;
            }

            if (*p != '}')
                vm_error(vm, "[printf] missing closing }.");

            if (index + 1 >= argc)
                vm_error(vm, "[printf] argument index out of range.");

            const char *arg = as_string(argv[index + 1]);

            // Render directly with color
            screen_print(
                vm->screen,
                arg,
                vm->screen->cursor_x,
                vm->screen->cursor_y,
                color);

            continue;
        }

        // Handle \n
        if (*p == '\\' && *(p + 1) == 'n')
        {
            vm->screen->cursor_x = 1;
            vm->screen->cursor_y += 6;
            p++;
            continue;
        }

        // Normal character
        char temp[2] = {*p, '\0'};
        screen_print(
            vm->screen,
            temp,
            vm->screen->cursor_x,
            vm->screen->cursor_y,
            vm->screen->text_color);
    }

    return NEW_NIL();
}

/**
 * @brief Prints a message to the console.
 *
 * This function takes a message as a string and prints it to the console.
 * It also takes an optional flag string that specifies the type of log message:
 *   - "e" for error log messages
 *   - "w" for warning log messages
 *
 * If no flag is provided, the message is simply printed to the console.
 *
 * @param vm The virtual machine instance.
 * @param argc The argument count; expects at least 1 argument.
 * @param argv The argument values; expects the first argument to be a string.
 * @return A nil value indicating completion.
 */
Value pi_log(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[log] expects message.");

    const char *msg = as_string(argv[0]);

    const char *flag = "i";

    if (argc >= 2 && IS_STRING(argv[1]))
        flag = AS_CSTRING(argv[1]);

    // Error log message
    if (strcmp(flag, "e") == 0)
        printf(ANSI_RED "%s" ANSI_RESET "\n", msg);
    // Warning log message
    else if (strcmp(flag, "w") == 0)
        printf(ANSI_YELLOW "%s" ANSI_RESET "\n", msg);
    // Normal log message
    else
        printf("%s\n", msg);

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
        vm_error(vm, "[key] expects at least one argument (string or number)");

    bool once = false;

    SDL_Scancode scancode;

    if (argc >= 2)
        once = as_bool(argv[1]);

    if (IS_STRING(argv[0]))
    {
        const char *keyname = as_string(argv[0]);
        scancode = get_keyCode(keyname);
        if (scancode == SDL_SCANCODE_UNKNOWN)
            vm_errorf(vm, "[key] Unknown key name: %s", keyname);
    }
    else if (IS_NUM(argv[0]))
        scancode = (SDL_Scancode)as_number(argv[0]);
    else
        vm_error(vm, "[key] Argument must be string or number");

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
            prev_pressed = false;

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
        vm_error(vm, "[input] expects a single string argument as a prompt.");

    PiString *prompt = AS_STRING(argv[0]);
    printf("%s", prompt->chars);
    fflush(stdout);

    char buffer[BUFFER_SIZE];
    if (!fgets(buffer, BUFFER_SIZE, stdin))
        vm_error(vm, "[input] Failed to read input.");

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
        vm_error(vm, "[open] expects a single string argument as a file path.");

    char *mode = "r";
    if (argc >= 2)
    {
        if (IS_STRING(argv[1]))
            mode = AS_CSTRING(argv[1]);
        else
            vm_error(vm, "[open] expects a string argument as a file mode.");
    }

    PiString *path = AS_STRING(argv[0]);
    FILE *file = fopen(path->chars, mode);

    if (!file)
        vm_errorf(vm, "[open] Failed to open file: %s", path->chars);

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
        vm_error(vm, "[read] expects a single file handler as argument.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        vm_error(vm, "[read] File is closed.");

    size_t buffer_size = BUFFER_SIZE;
    size_t capacity = buffer_size;
    size_t length = 0;

    char *content = malloc(capacity);
    if (!content)
        vm_error(vm, "[read] Out of memory.");

    while (!feof(file->fp))
    {
        if (length + buffer_size > capacity)
        {
            capacity *= 2;
            char *new_content = realloc(content, capacity);
            if (!new_content)
            {
                free(content);
                vm_error(vm, "[read] Out of memory during read.");
            }
            content = new_content;
        }

        size_t bytes = fread(content + length, 1, buffer_size, file->fp);
        if (ferror(file->fp))
        {
            free(content);
            vm_errorf(vm, "[read] Failed to read file: %s", file->filename);
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
        vm_error(vm, "[write] expects a file handler and a string as arguments.");

    if (!IS_STRING(argv[1]))
        vm_error(vm, "[write] second argument must be a string.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        vm_error(vm, "[write] File is closed.");

    char *str = AS_CSTRING(argv[1]);

    size_t written = fwrite(str, 1, strlen(str), file->fp);

    if (written < strlen(str) || ferror(file->fp))
        vm_errorf(vm, "[write] Failed to write to file: %s", file->filename);

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
        vm_error(vm, "[seek] expects a file handler and a number as arguments.");

    ObjFile *file = AS_FILE(argv[0]);

    if (file->closed)
        vm_error(vm, "[seek] File is closed.");

    if (!IS_NUM(argv[1]))
        vm_error(vm, "[seek] second argument must be a number.");

    long pos = as_number(argv[1]);
    if (fseek(file->fp, pos, SEEK_SET) != 0)
        vm_errorf(vm, "[seek] Failed to seek in file: %s", file->filename);

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
        vm_error(vm, "[close] expects a file handler as argument.");

    ObjFile *file = AS_FILE(argv[0]);

    if (fclose(file->fp) != 0)
        vm_errorf(vm, "[close] Failed to close file: %s", file->filename);

    file->closed = true;
    return NEW_BOOL(true);
}
