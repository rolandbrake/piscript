#include <math.h>   // For rounding
#include <stdlib.h> // For memory allocation

#include "pi_plot.h"
#include "../screen.h"
#include "../common.h"

/**
 * Draws a pixel on the screen at the specified coordinates with a given color and optional alpha transparency.
 *
 * This function expects either 3 or 4 numeric arguments: the x and y coordinates, the color index,
 * and optionally the alpha transparency value. If the alpha value is not provided, it defaults to 1.0.
 * The alpha value is clamped between 0.0 and 1.0.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects 3 or 4).
 * @param argv The arguments: x, y, color and optionally alpha.
 * @return A nil value indicating completion.
 */

Value pi_pixel(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3 ||
        argv[0].type != VAL_NUM ||
        argv[1].type != VAL_NUM ||
        argv[2].type != VAL_NUM ||
        (argc == 4 && argv[3].type != VAL_NUM))
    {
        error("[pixel] expects 3 or 4 numeric arguments: x, y, color [, alpha].");
    }

    int x = (int)round(AS_NUM(argv[0]));
    int y = (int)round(AS_NUM(argv[1]));
    int color = (int)round(AS_NUM(argv[2]));
    float alpha = (argc == 4) ? AS_NUM(argv[3]) : 1.0f;

    if (alpha < 0.0f)
        alpha = 0.0f;
    if (alpha > 1.0f)
        alpha = 1.0f;

    Screen *screen = vm->screen;
    set_pixel_alpha(screen, x, y, color, alpha);
    return NEW_NIL();
}

/**
 * Draws a line on the screen between two points with a specified color.
 *
 * This function expects at least 5 numeric arguments: the x and y coordinates of the start
 * point, the x and y coordinates of the end point, and the color index. The color index
 * is wrapped within 32.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects at least 5).
 * @param argv The arguments: x1, y1, x2, y2, color.
 * @return A nil value indicating completion.
 */

Value pi_line(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 ||
        argv[0].type != VAL_NUM ||
        argv[1].type != VAL_NUM ||
        argv[2].type != VAL_NUM ||
        argv[3].type != VAL_NUM)
        error("[line] expects four numeric arguments at least.");

    int x1 = (int)round(AS_NUM(argv[0]));
    int y1 = (int)round(AS_NUM(argv[1]));
    int x2 = (int)round(AS_NUM(argv[2]));
    int y2 = (int)round(AS_NUM(argv[3]));
    int color = ((int)round(AS_NUM(argv[4])) % 32);

    Screen *screen = vm->screen; // Assume vm has a Screen reference

    draw_line(screen, x1, y1, x2, y2, color);

    return NEW_NIL();
}

/**
 * Updates the screen by applying all pending drawing operations.
 *
 * This function is intended to be called after any series of drawing functions
 * to ensure that the changes are rendered on the screen.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (not used in this function).
 * @param argv The arguments (not used in this function).
 * @return A nil value indicating completion.
 */

Value pi_draw(vm_t *vm, int argc, Value *argv)
{
    screen_update(vm->screen);
    return NEW_NIL();
}

/**
 * Clears the screen with a specified color.
 *
 * This function takes an optional single numeric argument which specifies the color index
 * to use when clearing the screen. If no argument is provided, the default color index of 12
 * will be used. The color index is wrapped within 32.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (0 or 1).
 * @param argv The arguments: color (optional).
 * @return A nil value indicating completion.
 */
Value pi_clear(vm_t *vm, int argc, Value *argv)
{
    int color = 12;
    if (argc == 1 && argv[0].type == VAL_NUM)
        color = (int)round(AS_NUM(argv[0])) % 32;
    screen_clear(vm->screen, color);
    return NEW_NIL();
}

/**
 * Draws a circle on the screen.
 *
 * This function expects at least four numeric arguments: the x and y coordinates
 * of the circle's center, the radius, and the color index. The color index is
 * wrapped within 32. Optionally, a fifth argument can be provided to specify
 * whether the circle should be filled or just outlined.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (4 or 5).
 * @param argv The arguments: x, y, radius, color, filled (optional).
 * @return A nil value indicating completion.
 */

Value pi_circ(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 ||
        argv[0].type != VAL_NUM ||
        argv[1].type != VAL_NUM ||
        argv[2].type != VAL_NUM ||
        argv[3].type != VAL_NUM)
        error("[circ] expects four numeric arguments at least.");

    int x = (int)round(AS_NUM(argv[0]));
    int y = (int)round(AS_NUM(argv[1]));
    int radius = (int)round(AS_NUM(argv[2]));
    int color = ((int)round(AS_NUM(argv[3])) % 32);

    Screen *screen = vm->screen; // Assume vm has a Screen reference

    bool filled = false;
    if (argc > 4)
        filled = as_bool(argv[4]);
    if (filled)
        draw_fillCircle(screen, x, y, radius, color);
    else
        draw_circle(screen, x, y, radius, color);

    return NEW_NIL();
}

/*************  ✨ Windsurf Command ⭐  *************/
/**
 * Draws a rectangle on the screen.
 *
 * This function takes five numeric arguments: the x and y coordinates of the top-left
 * corner, the width and height of the rectangle, and the color index. The color index
 * is wrapped within 32. Optionally, a sixth argument can be provided to specify whether
 * the rectangle should be filled or just outlined.
 *
 * @param vm The virtual machine instance.

 * @param argc The number of arguments (5 or 6).
 * @param argv The arguments: x, y, width, height, color, filled (optional).
 * @return A nil value indicating completion.
 */
Value pi_rect(vm_t *vm, int argc, Value *argv)
{
    if (argc < 5 ||
        argv[0].type != VAL_NUM ||
        argv[1].type != VAL_NUM ||
        argv[2].type != VAL_NUM ||
        argv[3].type != VAL_NUM ||
        argv[4].type != VAL_NUM)
        error("[rect] expects five numeric arguments.");

    bool filled = false;
    if (argc > 5)
        filled = as_bool(argv[5]);

    int x = (int)round(AS_NUM(argv[0]));
    int y = (int)round(AS_NUM(argv[1]));
    int width = (int)round(AS_NUM(argv[2]));
    int height = (int)round(AS_NUM(argv[3]));
    int color = ((int)round(AS_NUM(argv[4])) % 32);

    Screen *screen = vm->screen; // Assume vm has a Screen reference
    if (filled)
        draw_fillRect(screen, x, y, width, height, color);
    else
        draw_rect(screen, x, y, width, height, color);
    return NEW_NIL();
}

/**
 * Draws a polygon on the screen using a list of points and a color index.
 *
 * This function takes two arguments: a list of points and a color index. The color index
 * is wrapped within 32. Optionally, a third argument can be provided to specify whether the
 * polygon should be filled or just outlined.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (2 or 3).
 * @param argv The arguments: points (list), color (int), filled (bool, optional).
 * @return A nil value indicating completion.
 */
Value pi_poly(vm_t *vm, int argc, Value *argv)
{    
    if (argc < 2 || !IS_LIST(argv[0]) || !IS_NUM(argv[1]))
        error("[poly] expects a list of points and a color index.");

    Screen *screen = vm->screen; // Assume vm has a Screen reference
    list_t *points = AS_LIST(argv[0])->items;

    int color = ((int)round(AS_NUM(argv[1])) % 32);

    bool filled = false;
    if (argc > 2)
        filled = as_bool(argv[2]);

    if (filled)
        draw_fillPolygon(screen, points, color);
    else
        draw_polygon(screen, points, color);

    return NEW_NIL();
}

Value pi_sprite(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        error("[sprite] expects at least one argument.");

    Screen *screen = vm->screen;
    int sprite_count = screen->sprite_count;
    Sprite *sprites = screen->sprites;

    int x = (int)AS_NUM(argv[1]);
    int y = (int)AS_NUM(argv[2]);

    // Check if the first argument is a 2D list of color values
    if (IS_LIST(argv[0]))
    {
        list_t *colors = AS_LIST(argv[0])->items;

        // Get the dimensions of the sprite
        int width = list_size(list_getAt(colors, 0)),
            height = list_size(colors);

        // Copy the color values into the sprite
        int index = 0;
        for (int i = 0; i < list_size(colors); i++)
        {
            Value value = *(Value *)list_getAt(colors, i);
            list_t *row = AS_LIST(value)->items;
            for (int j = 0; j < list_size(row); j++)
            {
                Value color = *(Value *)list_getAt(row, j);
                if (IS_NUM(color))
                    error("[sprite] expected a numeric color value.");
                if (AS_NUM(color) != 0)
                    set_pixel(screen, j + x, i + y, AS_NUM(color));
            }
        }
    }
    // Check if the first argument is an index of a sprite
    else if (IS_NUM(argv[0]))
    {
        int index = (int)AS_NUM(argv[0]);

        if (index < 0 || index >= sprite_count)
            error("[sprite] invalid sprite index.");

        Sprite *sprite = &sprites[index];

        // Draw the sprite on the screen
        for (int i = 0; i < sprite->height; i++)
            for (int j = 0; j < sprite->width; j++)
            {
                uint8_t color = sprite->pixels[i * sprite->width + j];
                if (color != 0)
                    set_pixel(screen, j + x, i + y, color);
            }
    }
    else
        error("[sprite] expected a 2D list of color values or a sprite index.");

    return NEW_NIL();
}

/**
 * @brief Returns the palette index (0-31) of the pixel at position (x, y)
 *        by matching its RGBA color with the palette.
 *
 * @param vm The virtual machine.
 * @param argc Number of arguments (should be 2).
 * @param argv Arguments: x and y coordinates.
 * @return Palette index as a numeric value.
 */
Value pi_color(vm_t *vm, int argc, Value *argv)
{
    if (argc != 2)
        error("[color] expects exactly two arguments (x, y).");

    if (!is_numeric(argv[0]) || !is_numeric(argv[1]))
        error("[color] arguments must be numeric.");

    int x = (int)as_number(argv[0]);
    int y = (int)as_number(argv[1]);

    if (x < 0 || x >= 128 || y < 0 || y >= 128)
        error("[color] pixel coordinates out of bounds (0-127).");

    int index = y * 128 + x;
    Uint32 pixel_color = vm->screen->pixels[index];

    int palette_index = get_colorIndex(pixel_color);
    return NEW_NUM((double)palette_index);
}
