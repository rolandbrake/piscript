/* screen.c */
#include "screen.h"
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "pi_value.h"
#include "common.h"

#define CHAR_COUNT 95 // Number of characters in the array

const char *characters[CHAR_COUNT] = {

    "0000 0000 0000 0000 0000 0000", // space
    "0010 0010 0010 0000 0010 0000", // !
    "1010 1010 0000 0000 0000 0000", // "
    "1010 1110 1010 1110 1010 0000", // #
    "1110 1100 0110 1110 0100 0000", // $
    "1010 0010 0100 1000 1010 0000", // %
    "1100 1100 0110 1010 1110 0000", // &
    "0010 0010 0000 0000 0000 0000", // '
    "0010 0100 0100 0100 0010 0000", // (
    "0100 0010 0010 0010 0100 0000", // )
    "0000 1010 0100 1010 0000 0000", // *
    "0000 0100 1110 0100 0000 0000", // +
    "0000 0000 0000 0010 0100 0000", // ,
    "0000 0000 1110 0000 0000 0000", // -
    "0000 0000 0000 0110 0110 0000", // .
    "0000 0010 0100 0100 1000 0000", // /

    "1110 1010 1010 1010 1110 0000", // 0
    "1100 0100 0100 0100 1110 0000", // 1
    "1110 0010 1110 1000 1110 0000", // 2
    "1110 0010 0110 0010 1110 0000", // 3
    "1010 1010 1110 0010 0010 0000", // 4
    "1110 1000 1110 0010 1110 0000", // 5
    "1000 1000 1110 1010 1110 0000", // 6
    "1110 0010 0010 0010 0010 0000", // 7
    "1110 1010 1110 1010 1110 0000", // 8
    "1110 1010 1110 0010 1110 0000", // 9

    "0000 0000 0010 0000 0010 0000", // :
    "0000 0010 0000 0010 0100 0000", // ;
    "0010 0100 1000 0100 0010 0000", // <
    "0000 0110 0000 0110 0000 0000", // =
    "1000 0100 0010 0100 1000 0000", // >
    "1110 0010 0110 0000 0100 0000", // ?
    "0100 1010 1010 1000 0110 0000", // @

    "1110 1010 1110 1010 1010 0000", // A
    "1110 1010 1100 1010 1110 0000", // B
    "0110 1000 1000 1000 0110 0000", // C
    "1100 1010 1010 1010 1110 0000", // D
    "1110 1000 1100 1000 1110 0000", // E
    "1110 1000 1100 1000 1000 0000", // F
    "0110 1000 1000 1010 1110 0000", // G
    "1010 1010 1110 1010 1010 0000", // H
    "1110 0100 0100 0100 1110 0000", // I
    "1110 0100 0100 0100 1100 0000", // J
    "1010 1010 1100 1010 1010 0000", // K
    "1000 1000 1000 1000 1110 0000", // L
    "1110 1110 1010 1010 1010 0000", // M
    "1100 1010 1010 1010 1010 0000", // N
    "0110 1010 1010 1010 1100 0000", // O
    "1110 1010 1110 1000 1000 0000", // P
    "0100 1010 1010 1100 0110 0000", // Q
    "1110 1010 1100 1010 1010 0000", // R
    "0110 1000 1110 0010 1100 0000", // S
    "1110 0100 0100 0100 0100 0000", // T
    "1010 1010 1010 1010 1100 0000", // U
    "1010 1010 1010 1010 0100 0000", // V
    "1010 1010 1010 1110 1110 0000", // W
    "1010 1010 0100 1010 1010 0000", // X
    "1010 1010 1110 0010 1110 0000", // Y
    "1110 0010 0100 1000 1110 0000", // Z

    "0110 0100 0100 0100 0110 0000", // [
    "0000 1000 0100 0100 0010 0000", // backslash
    "0110 0010 0010 0010 0110 0000", // ]
    "0000 0100 1010 0000 0000 0000", // ^
    "0000 0000 0000 0000 0110 0000", // _
    "0000 0100 0010 0000 0000 0000", // `

    "0000 1110 1010 1110 1010 0000", // a
    "0000 1100 1100 1010 1110 0000", // b
    "0000 1110 1000 1000 1110 0000", // c
    "0000 1100 1010 1010 1100 0000", // d
    "0000 1110 1100 1000 1110 0000", // e
    "0000 1110 1100 1000 1000 0000", // f
    "0000 1110 1000 1010 1110 0000", // g
    "0000 1010 1010 1110 1010 0000", // h
    "0000 1110 0100 0100 1110 0000", // i
    "0000 1110 0100 0100 1100 0000", // j
    "0000 1010 1100 1010 1010 0000", // k
    "0000 1000 1000 1000 1110 0000", // l
    "0000 1110 1110 1010 1010 0000", // m
    "0000 1100 1010 1010 1010 0000", // n
    "0000 0110 1010 1010 1100 0000", // o
    "0000 1110 1010 1110 1000 0000", // p
    "0000 0100 1010 1100 0110 0000", // q
    "0000 1110 1010 1100 1010 0000", // r
    "0000 0110 1000 0010 1100 0000", // s
    "0000 1110 0100 0100 0100 0000", // t
    "0000 1010 1010 1010 0110 0000", // u
    "0000 1010 1010 1010 0100 0000", // v
    "0000 1010 1010 1110 1110 0000", // w
    "0000 1010 0100 0100 1010 0000", // x
    "0000 1010 1110 0010 1110 0000", // y
    "0000 1110 0010 0100 1110 0000", // z

    "0110 0100 1100 0100 0110 0000", // {
    "0000 0010 0010 0010 0010 0000", // |
    "1100 0100 0110 0100 1100 0000", // }
    "0000 0000 0010 1110 1000 0000", // ~

};

// colors palette (extended)
const SDL_Color palette[PALETTE_SIZE] = {
    {0, 0, 0, 255},     // Black (0)
    {29, 43, 83, 255},  // Dark Blue (1)
    {126, 37, 83, 255}, // Dark Magenta (2)
    {0, 135, 81, 255},  // Dark Green (3)
    {171, 82, 54, 255}, // Brown (4)
    {95, 87, 79, 255},  // Dark Gray (5)
    // {194, 195, 199, 255}, // Light Gray
    {255, 255, 255, 255}, // White (6)
    {255, 241, 232, 255}, // Very Light Pink (7)
    {255, 0, 77, 255},    // Bright Red (8)
    {255, 163, 0, 255},   // Bright Orange (9)
    {255, 236, 39, 255},  // Bright Yellow (10)
    {0, 228, 54, 255},    // Bright Green  (11)
    {41, 173, 255, 255},  // Bright Blue (12)
    {131, 118, 156, 255}, // Soft Purple (13)
    {255, 119, 168, 255}, // Bright Pink (14)
    {255, 204, 170, 255}, // Peach (15)
    {41, 24, 20, 255},    // Dark Brown (16)
    {17, 29, 53, 255},    // Navy Blue (17)
    {66, 33, 54, 255},    // Deep Purple (18)
    {18, 83, 89, 255},    // Teal (19)
    {116, 47, 41, 255},   // Rust Red (20)
    {73, 51, 59, 255},    // Muted Purple (21)
    {162, 136, 121, 255}, // Warm Gray (22)
    {243, 239, 125, 255}, // Pale Lime (23)
    {190, 18, 80, 255},   // Dark Pink (24)
    {255, 108, 36, 255},  // Orange Red (25)
    {168, 231, 46, 255},  // Lime Green (26)
    {0, 181, 67, 255},    // Emerald Green (27)
    {6, 90, 181, 255},    // Cobalt Blue (28)
    {117, 70, 101, 255},  // Dusky Purple (29)
    {255, 110, 89, 255},  // Coral (30)
    {255, 157, 129, 255}, // Light Salmon (31)
};

static Uint32 colors[NUM_COLORS]; // Precomputed colors

Screen *screen_init(Color color)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }

    Screen *screen = malloc(sizeof(Screen));
    if (!screen)
        return NULL;

    screen->window = SDL_CreateWindow("PI-SHELL",
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

    SDL_RenderSetLogicalSize(screen->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

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

    screen->offset_x = 0;
    screen->offset_y = 0;
    screen->cursor_x = 0;
    screen->cursor_y = 0;

    screen_clear(screen, color);
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
void screen_clear(Screen *screen, Color color)
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
void inline set_pixel(Screen *screen, int x, int y, Color color)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

    if ((unsigned)x < SCREEN_WIDTH && (unsigned)y < SCREEN_HEIGHT)
        screen->pixels[y * SCREEN_WIDTH + x] = colors[color % PALETTE_SIZE];
}

void inline set_pixel_alpha(Screen *screen, int x, int y, Color color_index, double alpha)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

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

void set_pixel_shaded(Screen *screen, int x, int y, Color color, float brightness)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

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
void draw_line(Screen *screen, int x0, int y0, int x1, int y1, Color color)
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

void draw_rect(Screen *screen, int x, int y, int w, int h, Color color)
{
    draw_line(screen, x, y, x + w, y, color);
    draw_line(screen, x, y, x, y + h, color);
    draw_line(screen, x + w, y, x + w, y + h, color);
    draw_line(screen, x, y + h, x + w, y + h, color);
}

void draw_fillRect(Screen *screen, int x, int y, int w, int h, Color color)
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
void draw_circle(Screen *screen, int x0, int y0, int radius, Color color)
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

void draw_fillCircle(Screen *screen, int x0, int y0, int radius, Color color)
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
void draw_polygon(Screen *screen, list_t *points, Color color)
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

void draw_fillPolygon(Screen *screen, list_t *points, Color color)
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

void draw_matrix(Screen *screen, int x, int y, int w, int h, uint8_t matrix[h][w])
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            set_pixel(screen, x + i, y + j, matrix[j][i]);
}

void screen_print(Screen *screen, const char *text, int x, int y, Color color)
{
    screen->cursor_x = x;
    screen->cursor_y = y;

    for (const char *c = text; *c; c++)
    {
        unsigned char ch = (unsigned char)*c;

        // ASCII range supported by the font
        if (ch < 32 || ch > 126)
            continue;

        int index = ch - 32;

        if (index >= CHAR_COUNT)
            continue;

        const char *rows = characters[index];

        int row = 0;
        int col = 0;

        for (int i = 0; rows[i] != '\0'; i++)
        {
            if (rows[i] == ' ')
            {
                row++;
                col = 0;
                continue;
            }

            if (rows[i] == '1')
            {
                set_pixel(
                    screen,
                    screen->cursor_x + col,
                    screen->cursor_y + row,
                    color);
            }

            col++;
        }

        // Advance cursor
        if (screen->cursor_x + 4 >= SCREEN_WIDTH)
        {
            screen->cursor_x = 1;
            screen->cursor_y += 6;
        }
        else
        {
            screen->cursor_x += 4;
        }
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
