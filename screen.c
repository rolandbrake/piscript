/* screen.c */
#include "screen.h"
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "pi_value.h"
#include "common.h"

// #define CHAR_COUNT 52 // Number of characters in the array
#define CHAR_COUNT 77

const char *characters[CHAR_COUNT] = {
    "000000 011110 110110 110110 111010 000000", // a
    "110000 111100 110110 110110 111100 000000", // b
    "000000 011100 110000 110000 011100 000000", // c
    "000110 011110 110110 110110 011110 000000", // d
    "000000 011100 101110 111000 011100 000000", // e
    "001100 011000 011100 011000 011000 000000", // f
    "000000 011110 110110 111110 000110 011100", // g
    "110000 111100 110110 110110 110110 000000", // h
    "001100 000000 011100 001100 011110 000000", // i
    "000110 000000 000110 110110 011100 000000", // j
    "110000 110110 111100 110110 110110 000000", // k
    "011000 011000 011000 011000 001100 000000", // l
    "000000 111100 111110 101010 101010 000000", // m
    "000000 111100 110110 110110 110110 000000", // n
    "000000 011100 110110 110110 011100 000000", // o
    "000000 111100 110110 111100 110000 000000", // p
    "000000 011110 110110 011110 000110 000000", // q
    "000000 111100 110110 110000 110000 000000", // r
    "000000 011110 111000 001110 111100 000000", // s
    "001100 011110 001100 001100 001100 000000", // t
    "000000 110110 110110 110110 111100 000000", // u
    "000000 110110 110110 110100 011000 000000", // v
    "000000 110110 110110 111110 010100 000000", // w
    "000000 011010 001100 011010 011010 000000", // x
    "000000 110110 111110 000110 011110 000000", // y
    "000000 011110 001100 011000 011110 000000", // z
    "011100 110110 111110 110110 110110 000000", // A
    "111100 110110 111110 110110 111110 000000", // B
    "011100 110110 110000 110110 011100 000000", // C
    "111100 110110 110110 110110 111100 000000", // D
    "111110 110000 111100 110000 111110 000000", // E
    "111110 110000 111100 110000 110000 000000", // F
    "011110 110000 110110 110110 011100 000000", // G
    "110110 110110 111110 110110 110110 000000", // H
    "011110 001100 001100 001100 011110 000000", // I
    "011110 001100 001100 101100 011100 000000", // J
    "110110 111110 111000 110110 110110 000000", // K
    "110000 110000 110000 110000 111100 000000", // L
    "110110 111110 111110 110110 110110 000000", // M
    "111100 110110 110110 110110 110110 000000", // N
    "011100 110110 110110 110110 011100 000000", // O
    "111100 110110 110110 111100 110000 000000", // P
    "011100 110110 110110 111100 011110 000000", // Q
    "111100 110110 110110 111000 110110 000000", // R
    "011110 111000 011110 000110 011110 000000", // S
    "111110 011000 011000 011000 011000 000000", // T
    "110110 110110 110110 110110 111100 000000", // U
    "110110 110110 110110 110110 011100 000000", // V
    "110110 110110 111110 111110 110110 000000", // W
    "110110 110110 011100 110110 110110 000000", // X
    "110110 110110 111110 000110 111110 000000", // Y
    "111110 001100 011000 110000 111110 000000", // Z
    "011110 110111 111011 110011 011110 000000", // 0
    "001100 011100 001100 001100 011110 000000", // 1
    "111110 000110 111110 110000 111110 000000", // 2
    "111100 000110 011110 000110 111110 000000", // 3
    "110110 110110 111110 000110 000110 000000", // 4
    "111110 110000 111110 000110 111110 000000", // 5
    "111110 110000 111110 110110 111110 000000", // 6
    "111110 000110 000110 000110 000110 000000", // 7
    "111110 110110 111110 110110 111110 000000", // 8
    "111110 110110 111110 000110 000110 000000", // 9
    "000000 000000 000000 000000 000000 000000", // Space
    "011000 011000 011000 000000 011000 000000", // !
    "000000 000000 010100 000000 000000 000000", // "
    "010100 111110 010100 111110 010100 000000", // #
    "011110 101000 011100 001010 111100 001000", // $
    "110011 110110 001100 011011 110011 000000", // %
    "011100 010100 011000 010100 001010 000000", // &
    "000000 000000 000000 010000 100000 000000", // '
    "001000 010100 100010 000000 000000 000000", // (
    "100010 010100 001000 000000 000000 000000", // )
    "000000 011100 000000 011100 000000 000000", // +
    "000000 000000 000000 000000 011000 000000", // ,
    "000000 011110 000000 000000 000000 000000", // -
    "000001 000010 000100 001000 010000 000000", // /
    "000000 000000 000000 011000 011000 000000", // .
};

// PICO-8 palette colors (extended)
const SDL_Color palette[NUM_COLORS] = {
    {0, 0, 0, 255},     // Black
    {29, 43, 83, 255},  // Dark Blue
    {126, 37, 83, 255}, // Dark Magenta
    {0, 135, 81, 255},  // Dark Green
    {171, 82, 54, 255}, // Brown
    {95, 87, 79, 255},  // Dark Gray
    // {194, 195, 199, 255}, // Light Gray
    {255, 255, 255, 255}, // White
    {255, 241, 232, 255}, // Very Light Pink
    {255, 0, 77, 255},    // Bright Red
    {255, 163, 0, 255},   // Bright Orange
    {255, 236, 39, 255},  // Bright Yellow
    {0, 228, 54, 255},    // Bright Green
    {41, 173, 255, 255},  // Bright Blue
    {131, 118, 156, 255}, // Soft Purple
    {255, 119, 168, 255}, // Bright Pink
    {255, 204, 170, 255}, // Peach
    {41, 24, 20, 255},    // Dark Brown
    {17, 29, 53, 255},    // Navy Blue
    {66, 33, 54, 255},    // Deep Purple
    {18, 83, 89, 255},    // Teal
    {116, 47, 41, 255},   // Rust Red
    {73, 51, 59, 255},    // Muted Purple
    {162, 136, 121, 255}, // Warm Gray
    {243, 239, 125, 255}, // Pale Lime
    {190, 18, 80, 255},   // Dark Pink
    {255, 108, 36, 255},  // Orange Red
    {168, 231, 46, 255},  // Lime Green
    {0, 181, 67, 255},    // Emerald Green
    {6, 90, 181, 255},    // Cobalt Blue
    {117, 70, 101, 255},  // Dusky Purple
    {255, 110, 89, 255},  // Coral
    {255, 157, 129, 255}, // Light Salmon
};

static Uint32 colors[PALETTE_SIZE]; // Precomputed colors

Screen *screen_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }

    Screen *screen = malloc(sizeof(Screen));
    if (!screen)
        return NULL;

    screen->window = SDL_CreateWindow("PiScript Console",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE,
                                      SDL_WINDOW_SHOWN);
    if (!screen->window)
    {
        free(screen);
        return NULL;
    }

#ifndef __EMSCRIPTEN__
    // Load icon (BMP format)
    SDL_Surface *icon = SDL_LoadBMP("./pi.bmp");
    if (!icon)
        SDL_Log("Failed to load icon: %s", SDL_GetError());
    else
    {
        SDL_SetWindowIcon(screen->window, icon); // Set the window icon
        SDL_FreeSurface(icon);                   // Free the surface after setting
    }

#endif

    screen->renderer = SDL_CreateRenderer(screen->window, -1,
                                          SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_RenderSetScale(screen->renderer, SCALE, SCALE);

    screen->texture = SDL_CreateTexture(screen->renderer,
                                        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                        SCREEN_WIDTH, SCREEN_HEIGHT);

    screen->pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));
    if (!screen->pixels)
    {
        SDL_DestroyRenderer(screen->renderer);
        SDL_DestroyWindow(screen->window);
        free(screen);
        return NULL;
    }

    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    for (int i = 0; i < PALETTE_SIZE; i++)
        colors[i] = SDL_MapRGBA(format, palette[i].r, palette[i].g, palette[i].b, 255);

    SDL_FreeFormat(format);

    SDL_ShowCursor(SDL_DISABLE);

    memset(screen->pixels, 12, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));

    screen->cursor_x = 0;
    screen->cursor_y = 0;

    screen->sprite_count = 0;

    screen_clear(screen, 12);
    screen_update(screen);
    return screen;
}

/**
 * Frees the screen and related resources.
 *
 * This function is responsible for freeing all the resources allocated for the
 * screen, including the window, renderer, texture and pixel data.
 *
 * @param screen The screen to free and close.
 */
void screen_close(Screen *screen)
{
    if (screen)
    {
        // Destroy the texture and renderer
        SDL_DestroyTexture(screen->texture);
        SDL_DestroyRenderer(screen->renderer);

        // Destroy the window
        SDL_DestroyWindow(screen->window);

        // Free the pixel data
        free(screen->pixels);

        // Free the screen instance
        free(screen);
    }

    // Quit SDL
    SDL_Quit();
}

/**
 * Updates the screen with the latest pixel data.
 *
 * This function updates the texture with the current pixel data
 * and renders it to the screen.
 *
 * @param screen The screen to update.
 */
void screen_update(Screen *screen)
{
    // Update the texture with the current pixel data
    SDL_UpdateTexture(screen->texture, NULL, screen->pixels, SCREEN_WIDTH * sizeof(Uint32));

    // Copy the texture to the rendering target
    SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);

    // Present the updated rendering to the screen
    SDL_RenderPresent(screen->renderer);
}

/**
 * Clears the screen by setting all pixels to the given color.
 *
 * This function fills the entire screen with the specified color from the
 * precomputed colors array and updates the screen display.
 *
 * @param screen The screen to clear.
 * @param color The color index from the palette to fill the screen with.
 */
void screen_clear(Screen *screen, int color)
{
    // Calculate the actual color value using the palette
    const Uint32 _color = colors[color % PALETTE_SIZE];
    // Total number of pixels on the screen
    const int size = SCREEN_WIDTH * SCREEN_HEIGHT;

    // Loop through the pixels in chunks of 4 for performance reasons
    for (int i = 0; i < size; i += 4)
    {
        screen->pixels[i] = _color;
        screen->pixels[i + 1] = _color;
        screen->pixels[i + 2] = _color;
        screen->pixels[i + 3] = _color;
    }

    // Update the screen with the new pixel data
    // screen_update(screen);

    // Reset cursor position to the top-left of the screen
    screen->cursor_x = 1;
    screen->cursor_y = 1;
}

/**
 * Sets a pixel on the screen to the given color.
 *
 * @param screen The screen to draw on.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color of the pixel, as an index into the palette.
 */
void inline set_pixel(Screen *screen, int x, int y, int color)
{
    if ((unsigned)x < SCREEN_WIDTH && (unsigned)y < SCREEN_HEIGHT)
        screen->pixels[y * SCREEN_WIDTH + x] = colors[color % PALETTE_SIZE];
}

void inline set_pixel_alpha(Screen *screen, int x, int y, int color_index, double alpha)
{
    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT)
        return;

    if (color_index < 0 || color_index >= PALETTE_SIZE)
        return;

    int index = y * SCREEN_WIDTH + x;

    Uint32 dst = screen->pixels[index];
    SDL_Color src_color = palette[color_index];
    Uint8 r1 = (dst >> 16) & 0xFF;
    Uint8 g1 = (dst >> 8) & 0xFF;
    Uint8 b1 = dst & 0xFF;

    Uint8 r2 = src_color.r;
    Uint8 g2 = src_color.g;
    Uint8 b2 = src_color.b;

    // Alpha blending: result = src * alpha + dst * (1 - alpha)
    Uint8 r = (Uint8)(r2 * alpha + r1 * (1 - alpha));
    Uint8 g = (Uint8)(g2 * alpha + g1 * (1 - alpha));
    Uint8 b = (Uint8)(b2 * alpha + b1 * (1 - alpha));

    // Set full alpha channel (255)
    screen->pixels[index] = (255 << 24) | (r << 16) | (g << 8) | b;
}

void set_pixel_shaded(Screen *screen, int x, int y, int color, float brightness)
{
    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT)
        return;

    if (color < 0 || color >= PALETTE_SIZE)
        return;

    SDL_Color src = palette[color];

    // Apply brightness to each RGB channel
    Uint8 r = (Uint8)(src.r * brightness);
    Uint8 g = (Uint8)(src.g * brightness);
    Uint8 b = (Uint8)(src.b * brightness);

    // Store pixel with full alpha (opaque)
    screen->pixels[y * SCREEN_WIDTH + x] = (255 << 24) | (r << 16) | (g << 8) | b;
}

/**
 * Draws a line on the screen using Bresenham's line algorithm.
 *
 * @param screen The screen to draw on.
 * @param x0 The starting x-coordinate of the line.
 * @param y0 The starting y-coordinate of the line.
 * @param x1 The ending x-coordinate of the line.
 * @param y1 The ending y-coordinate of the line.
 * @param color The color of the line.
 */
void draw_line(Screen *screen, int x0, int y0, int x1, int y1, int color)
{
    int dx = abs(x1 - x0);     // Difference in x
    int dy = abs(y1 - y0);     // Difference in y
    int sx = x0 < x1 ? 1 : -1; // Step direction for x
    int sy = y0 < y1 ? 1 : -1; // Step direction for y
    int err = dx - dy;         // Error term

    while (1)
    {
        set_pixel(screen, x0, y0, color); // Set pixel at current position
        if (x0 == x1 && y0 == y1)         // Check if the end of the line is reached
            break;
        int e2 = 2 * err; // Double the error term
        if (e2 > -dy)
        {
            err -= dy; // Adjust error term
            x0 += sx;  // Move in x direction
        }
        if (e2 < dx)
        {
            err += dx; // Adjust error term
            y0 += sy;  // Move in y direction
        }
    }
}

void draw_rect(Screen *screen, int x, int y, int w, int h, int color)
{
    draw_line(screen, x, y, x + w, y, color);
    draw_line(screen, x, y, x, y + h, color);
    draw_line(screen, x + w, y, x + w, y + h, color);
    draw_line(screen, x, y + h, x + w, y + h, color);
}

void draw_fillRect(Screen *screen, int x, int y, int w, int h, int color)
{
    for (int i = 0; i < h; i++)
        draw_line(screen, x, y + i, x + w, y + i, color);
}

/**
 * Draws a circle on the screen.
 *
 * @param screen The screen to draw on.
 * @param x0 The x-coordinate of the circle's center.
 * @param y0 The y-coordinate of the circle's center.
 * @param radius The radius of the circle.
 * @param color The color of the circle.
 */
void draw_circle(Screen *screen, int x0, int y0, int radius, int color)
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    // Loop until the x and y coordinates are equal
    while (y >= x)
    {
        // Plot the four points for this iteration of the loop
        set_pixel(screen, x0 + x, y0 + y, color);
        set_pixel(screen, x0 - x, y0 + y, color);
        set_pixel(screen, x0 + x, y0 - y, color);
        set_pixel(screen, x0 - x, y0 - y, color);
        set_pixel(screen, x0 + y, y0 + x, color);
        set_pixel(screen, x0 - y, y0 + x, color);
        set_pixel(screen, x0 + y, y0 - x, color);
        set_pixel(screen, x0 - y, y0 - x, color);

        // Increment x and adjust y accordingly
        x++;
        if (d > 0)
        {
            y--;
            d += 4 * (x - y) + 10;
        }
        else
            d += 4 * x + 6;
    }
}

void draw_fillCircle(Screen *screen, int x0, int y0, int radius, int color)
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (y >= x)
    {
        // For each pixel we will draw horizontal lines to fill the circle
        draw_line(screen, x0 - x, y0 + y, x0 + x, y0 + y, color);
        draw_line(screen, x0 - y, y0 + x, x0 + y, y0 + x, color);
        draw_line(screen, x0 - x, y0 - y, x0 + x, y0 - y, color);
        draw_line(screen, x0 - y, y0 - x, x0 + y, y0 - x, color);

        x++;

        // Check for decision parameter and correspondingly update d, x, y
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
    }
}
void draw_polygon(Screen *screen, list_t *points, int color)
{
    int size = list_size(points);

    int x0, y0, x1, y1;

    for (int i = 0; i < size - 2; i += 2)
    {
        x0 = (int)as_number(*(Value *)list_getAt(points, i));
        y0 = (int)as_number(*(Value *)list_getAt(points, i + 1));
        x1 = (int)as_number(*(Value *)list_getAt(points, i + 2));
        y1 = (int)as_number(*(Value *)list_getAt(points, i + 3));

        draw_line(screen, x0, y0, x1, y1, color);
    }

    x0 = (int)as_number(*(Value *)list_getAt(points, size - 2));
    y0 = (int)as_number(*(Value *)list_getAt(points, size - 1));
    x1 = (int)as_number(*(Value *)list_getAt(points, 0));
    y1 = (int)as_number(*(Value *)list_getAt(points, 1));

    draw_line(screen, x0, y0, x1, y1, color);
}

static int compare_num(const void *a, const void *b)
{
    int x = *(int *)a;
    int y = *(int *)b;
    return x - y;
}

void draw_fillPolygon(Screen *screen, list_t *points, int color)
{
    int size = list_size(points);
    if (size < 6)
        return; // Ensure at least 3 points (6 values: x,y,x,y,...)

    int minY = INT_MAX, maxY = INT_MIN;
    int *i_points = malloc(size * sizeof(int));

    // Convert list_t to an array & find min/max Y values
    for (int i = 0; i < size; i += 2)
    {
        int x = (int)as_number(*(Value *)list_getAt(points, i));
        int y = (int)as_number(*(Value *)list_getAt(points, i + 1));
        i_points[i] = x;
        i_points[i + 1] = y;

        if (y < minY)
            minY = y;
        if (y > maxY)
            maxY = y;
    }

    for (int y = minY; y <= maxY; y++)
    {
        int *inters = malloc(size * sizeof(int));
        int inters_count = 0;

        for (int i = 0; i < size; i += 2)
        {
            int x1 = i_points[i], y1 = i_points[i + 1];
            int x2 = i_points[(i + 2) % size], y2 = i_points[(i + 2) % size + 1];

            // Ignore horizontal edges
            if (y1 == y2)
                continue;

            // Check if scanline crosses this edge
            if (y >= MIN(y1, y2) && y < MAX(y1, y2))
            {
                // Fix rounding errors in intersection calculation
                double slope = (double)(x2 - x1) / (y2 - y1);
                int x_intersect = (int)round(x1 + (y - y1) * slope);

                inters[inters_count++] = x_intersect;
            }
        }

        // Sort intersections (to get correct left-right pairing)
        qsort(inters, inters_count, sizeof(int), compare_num);

        // Ensure intersections are always in pairs
        for (int i = 0; i < inters_count - 1; i += 2)
            for (int x = inters[i]; x <= inters[i + 1]; x++)
                set_pixel(screen, x, y, color);

        free(inters);
    }

    free(i_points);
}

void screen_print(Screen *screen, const char *text, int x, int y, int color)
{
    screen->cursor_x = x;
    screen->cursor_y = y;

    for (const char *c = text; *c; c++)
    {
        int index = 0;
        if (islower(*c))
            index = *c - 'a';
        else if (isupper(*c))
            index = *c - 'A' + 26;
        else if (isdigit(*c))
            index = *c - '0' + 52;
        else if (*c - ' ' <= 15)
            index = *c - ' ' + 62; // PiMap to indices starting from 63
        else
            continue;

        if (index >= (int)(sizeof(characters) / sizeof(characters[0])))
            continue;

        const char *rows = characters[index];

        int row = 0, col = 0;
        for (int i = 0; rows[i] != '\0'; i++)
        {
            if (rows[i] == ' ') // Skip spaces in the bitmap
            {
                row++;   // Move to the next row
                col = 0; // Reset column position
                continue;
            }

            if (rows[i] == '1')
                set_pixel(screen, screen->cursor_x + col, screen->cursor_y + row, color);

            col++;
        }

        if (screen->cursor_x + 12 >= SCREEN_WIDTH)
        {
            screen->cursor_y += 6;
            screen->cursor_x = 1;
        }
        else
            screen->cursor_x += 6;
    }
}

// screen.c
int get_colorIndex(Uint32 pixel_color)
{
    Uint8 r = (pixel_color >> 16) & 0xFF;
    Uint8 g = (pixel_color >> 8) & 0xFF;
    Uint8 b = (pixel_color >> 24) & 0xFF;

    int closest_index = 0;
    int min_diff = 256 * 256 * 4;

    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        int dr = (int)palette[i].r - r;
        int dg = (int)palette[i].g - g;
        int db = (int)palette[i].b - b;
        int diff = dr * dr + dg * dg + db * db;
        if (diff < min_diff)
        {
            min_diff = diff;
            closest_index = i;
        }
    }
    return closest_index;
}
