
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"

error_handlerFn global_errorHandler = NULL;

char *itos(int num)
{
    // Determine the length of the number
    int length = 0;
    int temp = num;

    // Handle the case of 0 explicitly
    if (temp == 0)
        length = 1;
    else
        // Calculate the length of the number
        while (temp != 0)
        {
            length++;
            temp /= 10;
        }

    // Allocate memory for the string (including space for the null terminator)
    char *str = (char *)malloc((length + 1) * sizeof(char));

    // Handle negative numbers
    if (num < 0)
    {
        str[0] = '-';
        num = -num;
    }

    // Convert each digit to a character
    int i = length - 1;
    do
    {
        str[i] = (num % 10) + '0';
        num /= 10;
        i--;
    } while (num != 0);

    // Null-terminate the string
    str[length] = '\0';

    return str;
}

void error(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    fflush(stdout); // Make sure stdout is flushed before printing to stderr
    fprintf(stderr, "[Error] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(EXIT_FAILURE);
}

extern const SDL_Color palette[PALETTE_SIZE];

/**
 * Finds the closest palette index for a given RGB color.
 *
 * This function takes RGB color values as separate arguments and
 * returns the index of the color in the palette that is closest to
 * the specified color.
 *
 * The distance between the specified color and the closest palette
 * color is calculated using the Euclidean distance metric.
 *
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @return The index of the closest palette color.
 */
int find_paletteColor(uint8_t r, uint8_t g, uint8_t b)
{
    int index = 0;
    int _dist = 256 * 256 * 3; // Initial distance value
    for (int i = 0; i < 32; i++)
    {
        // Calculate the Euclidean distance between the specified color
        // and the current palette color
        int dr = r - palette[i].r;
        int dg = g - palette[i].g;
        int db = b - palette[i].b;
        int dist = dr * dr + dg * dg + db * db;

        // If the current distance is less than the best distance found
        // so far, update the best distance and the index of the closest
        // palette color
        if (dist < _dist)
        {
            _dist = dist;
            index = i;
        }
    }
    return index;
}

/**
 * Sets a custom error handler function to be called on parsing errors.
 *
 * @param handler A function pointer to the custom error handler. If NULL,
 *                the default behavior (printing to stderr and exiting) is restored.
 */
void set_errorHandler(error_handlerFn handler)
{
    global_errorHandler = handler;
}
