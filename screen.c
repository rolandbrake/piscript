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
    "1110 1000 1110 1010 1110 0000", // 6
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

    // pi palette
    {2, 4, 6, 255},       // #020406 Soft Black (0)
    {34, 48, 96, 255},    // #223060 Deep Blue (1)
    {134, 42, 90, 255},   // #862a5a Dark Magenta (2)
    {4, 140, 90, 255},    // #048c5a Deep Green (3)
    {176, 88, 60, 255},   // #b0583c Warm Brown (4)
    {100, 92, 84, 255},   // #645c54 Charcoal Gray (5)
    {200, 202, 206, 255}, // #c8cace Cool Light Gray (6)
    {255, 244, 236, 255}, // #fff4ec Soft Ivory (7)
    {255, 12, 90, 255},   // #ff0c5a Vivid Red (8)
    {255, 168, 12, 255},  // #ffa80c Amber Orange (9)
    {255, 238, 48, 255},  // #ffee30 Warm Yellow (10)
    {12, 232, 64, 255},   // #0ce840 Neon Green (11)
    {48, 180, 255, 255},  // #30b4ff Sky Blue (12)
    {138, 124, 164, 255}, // #8a7ca4 Dusty Purple (13)
    {255, 126, 176, 255}, // #ff7eb0 Pink Rose (14)
    {255, 210, 176, 255}, // #ffd2b0 Light Peach (15)

    // pi palette extended
    {46, 28, 24, 255},    // #2e1c18 Dark Cocoa (16)
    {22, 36, 60, 255},    // #16243c Midnight Blue (17)
    {72, 38, 60, 255},    // #48263c Plum (18)
    {24, 90, 96, 255},    // #185a60 Ocean Teal (19)
    {124, 54, 48, 255},   // #7c3630 Brick Red (20)
    {80, 58, 66, 255},    // #503a42 Muted Mauve (21)
    {168, 142, 128, 255}, // #a88e80 Warm Stone (22)
    {246, 242, 136, 255}, // #f6f288 Pale Yellow (23)
    {198, 26, 92, 255},   // #c61a5c Deep Pink (24)
    {255, 116, 44, 255},  // #ff742c Orange Red (25)
    {176, 236, 56, 255},  // #b0ec38 Lime Yellow (26)
    {4, 188, 80, 255},    // #04bc50 Emerald (27)
    {12, 96, 190, 255},   // #0c60be Bright Cobalt (28)
    {124, 78, 110, 255},  // #7c4e6e Dusty Violet (29)
    {255, 120, 96, 255},  // #ff7860 Coral (30)
    {255, 164, 140, 255}, // #ffa48c Soft Salmon (31)

    // gameboy
    {208, 208, 88, 255},  // #d0d058 Light Olive (32)
    {160, 168, 64, 255},  // #a0a840 Olive (33)
    {112, 128, 40, 255},  // #708028 Dark Olive (34)
    {64, 80, 16, 255},    // #405010 Deep Olive (35)
    {51, 44, 80, 255},    // #332c50 Indigo Gray (36)
    {70, 135, 143, 255},  // #46878f Desaturated Cyan (37)
    {148, 227, 68, 255},  // #94e344 Bright Lime (38)
    {226, 243, 228, 255}, // #e2f3e4 Pale Mint (39)
    {33, 30, 32, 255},    // #211e20 Almost Black (40)
    {85, 85, 104, 255},   // #555568 Slate Gray (41)
    {160, 160, 139, 255}, // #a0a08b Dusty Gray (42)
    {233, 239, 236, 255}, // #e9efec Soft White (43)
    {124, 63, 88, 255},   // #7c3f58 Muted Rose (44)
    {235, 107, 111, 255}, // #eb6b6f Soft Red (45)
    {249, 168, 117, 255}, // #f9a875 Apricot (46)
    {255, 246, 211, 255}, // #fff6d3 Cream (47)

    // nes
    {194, 198, 48, 255},  // #c2c630 Yellow Green (48)
    {63, 161, 48, 255},   // #3fa130 Grass Green (49)
    {78, 88, 23, 255},    // #4e5817 Moss Green (50)
    {52, 43, 25, 255},    // #342b19 Dark Earth (51)
    {255, 211, 123, 255}, // #ffd37b Sand (52)
    {225, 135, 52, 255},  // #e18734 Pumpkin (53)
    {195, 78, 41, 255},   // #c34e29 Clay Red (54)
    {99, 72, 5, 255},     // #634805 Dark Gold (55)
    {193, 61, 149, 255},  // #c13d95 Fuchsia (56)
    {141, 21, 35, 255},   // #8d1523 Blood Red (57)
    {255, 255, 255, 255}, // #ffffff White (58)
    {173, 173, 203, 255}, // #adadcb Pale Lavender (59)
    {89, 110, 120, 255},  // #596e78 Blue Gray (60)
    {114, 179, 255, 255}, // #72b3ff Sky Blue (61)
    {61, 101, 182, 255},  // #3d65b6 Royal Blue (62)
    {36, 68, 73, 255},    // #244449 Dark Teal (63)

    // sega
    {43, 18, 13, 255},    // #2b120d Dark Brown (64)
    {159, 18, 17, 255},   // #9f1211 Deep Red (65)
    {252, 20, 0, 255},    // #fc1400 Bright Red (66)
    {252, 106, 0, 255},   // #fc6a00 Orange (67)
    {252, 252, 0, 255},   // #fcfc00 Yellow (68)
    {156, 12, 156, 255},  // #9c0c9c Purple (69)
    {255, 9, 157, 255},   // #ff099d Magenta (70)
    {0, 7, 44, 255},      // #00072c Very Dark Blue (71)
    {0, 0, 255, 255},     // #0000ff Blue (72)
    {103, 205, 252, 255}, // #67cdfc Light Blue (73)
    {0, 72, 73, 255},     // #004849 Teal (74)
    {0, 201, 8, 255},     // #00c908 Green (75)
    {82, 255, 0, 255},    // #52ff00 Lime (76)
    {173, 89, 80, 255},   // #ad5950 Muted Red Brown (77)
    {252, 180, 72, 255},  // #fcb448 Light Orange (78)
    {221, 217, 230, 255}, // #ddd9e6 Light Lavender Gray (79)
};

static Uint32 colors[NUM_COLORS]; // Precomputed palette colors

Uint32 screen_resolveColor(Uint32 color)
{
    if (color < PALETTE_SIZE)
        return colors[color];

    return color;
}

bool screen_colorFromNumber(double number, Uint32 *color)
{
    if (!isfinite(number) || number < 0.0 || number > 4294967295.0)
        return false;

    *color = (Uint32)round(number);
    return true;
}

bool screen_paletteColor(int index, Uint32 *color)
{
    if (color == NULL || index < 0 || index >= PALETTE_SIZE)
        return false;

    SDL_Color entry = palette[index];
    *color = ((Uint32)entry.a << 24) |
             ((Uint32)entry.r << 16) |
             ((Uint32)entry.g << 8) |
             (Uint32)entry.b;
    return true;
}

static Uint32 blend_color(Uint32 dst, Uint32 src, double opacity)
{
    if (opacity <= 0.0)
        return dst;
    if (opacity > 1.0)
        opacity = 1.0;

    double src_alpha = ((src >> 24) & 0xff) / 255.0;
    double alpha = src_alpha * opacity;
    if (alpha <= 0.0)
        return dst;

    Uint8 r1 = (dst >> 16) & 0xff;
    Uint8 g1 = (dst >> 8) & 0xff;
    Uint8 b1 = dst & 0xff;
    Uint8 r2 = (src >> 16) & 0xff;
    Uint8 g2 = (src >> 8) & 0xff;
    Uint8 b2 = src & 0xff;

    Uint8 r = (Uint8)(r2 * alpha + r1 * (1.0 - alpha));
    Uint8 g = (Uint8)(g2 * alpha + g1 * (1.0 - alpha));
    Uint8 b = (Uint8)(b2 * alpha + b1 * (1.0 - alpha));
    return (255u << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | b;
}

Screen *screen_init(Uint32 color)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }

    Screen *screen = malloc(sizeof(Screen));
    if (!screen)
        return NULL;

    screen->window = SDL_CreateWindow("PI-SCRIPT",
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
    screen->cursor_x = 1;
    screen->cursor_y = 1;
    screen->text_color = COLOR_WHITE;
    screen->dirty = true;
    screen->fullscreen = false;

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
    if (!screen->dirty)
        return;

    // Update the texture with the current pixel data
    SDL_UpdateTexture(screen->texture, NULL, screen->pixels, SCREEN_WIDTH * sizeof(Uint32));

    // Copy the texture to the rendering target
    SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);

    // Present the updated rendering to the screen
    SDL_RenderPresent(screen->renderer);
    screen->dirty = false;
}

void screen_toggleFullscreen(Screen *screen)
{
    if (!screen || !screen->window)
        return;

    bool next = !screen->fullscreen;
    Uint32 flags = next ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    if (SDL_SetWindowFullscreen(screen->window, flags) != 0)
    {
        SDL_Log("Failed to toggle fullscreen: %s", SDL_GetError());
        return;
    }

    screen->fullscreen = next;
    SDL_RenderSetLogicalSize(screen->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    screen->dirty = true;
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
void screen_clear(Screen *screen, Uint32 color)
{
    const Uint32 _color = screen_resolveColor(color);
    screen->clear_color = _color;

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
    screen->dirty = true;
}

/**
 * Sets a pixel on the screen to the given color.
 *
 * @param screen The screen to draw on.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color of the pixel, as an index into the palette.
 */
void inline set_pixel(Screen *screen, int x, int y, Uint32 color)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

    if ((unsigned)x < SCREEN_WIDTH && (unsigned)y < SCREEN_HEIGHT)
    {
        int index = y * SCREEN_WIDTH + x;
        screen->pixels[index] = blend_color(screen->pixels[index], screen_resolveColor(color), 1.0);
        screen->dirty = true;
    }
}

void inline set_pixelAlpha(Screen *screen, int x, int y, Uint32 color, double alpha)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT)
        return;

    int index = y * SCREEN_WIDTH + x;
    screen->pixels[index] = blend_color(screen->pixels[index], screen_resolveColor(color), alpha);
    screen->dirty = true;
}

void set_pixelShade(Screen *screen, int x, int y, Uint32 color, float brightness)
{
    x -= screen->offset_x;
    y -= screen->offset_y;

    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT)
        return;

    Uint32 src = screen_resolveColor(color);
    Uint8 alpha = (src >> 24) & 0xff;
    Uint8 r = (Uint8)(((src >> 16) & 0xff) * brightness);
    Uint8 g = (Uint8)(((src >> 8) & 0xff) * brightness);
    Uint8 b = (Uint8)((src & 0xff) * brightness);
    Uint32 shaded = ((Uint32)alpha << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | b;
    int index = y * SCREEN_WIDTH + x;
    screen->pixels[index] = blend_color(screen->pixels[index], shaded, 1.0);
    screen->dirty = true;
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
void draw_line(Screen *screen, int x0, int y0, int x1, int y1, Uint32 color)
{
    x0 -= screen->offset_x;
    y0 -= screen->offset_y;
    x1 -= screen->offset_x;
    y1 -= screen->offset_y;

    // Fast trivial reject if the whole segment is outside one side.
    if ((x0 < 0 && x1 < 0) ||
        (x0 >= SCREEN_WIDTH && x1 >= SCREEN_WIDTH) ||
        (y0 < 0 && y1 < 0) ||
        (y0 >= SCREEN_HEIGHT && y1 >= SCREEN_HEIGHT))
        return;

    Uint32 packed = screen_resolveColor(color);
    int dx = abs(x1 - x0);     // Difference in x
    int dy = abs(y1 - y0);     // Difference in y
    int sx = x0 < x1 ? 1 : -1; // Step direction for x
    int sy = y0 < y1 ? 1 : -1; // Step direction for y
    int err = dx - dy;         // Error term
    bool wrote = false;

    while (1)
    {
        if ((unsigned)x0 < SCREEN_WIDTH && (unsigned)y0 < SCREEN_HEIGHT)
        {
            int index = y0 * SCREEN_WIDTH + x0;
            screen->pixels[index] = blend_color(screen->pixels[index], packed, 1.0);
            wrote = true;
        }
        if (x0 == x1 && y0 == y1) // Check if the end of the line is reached
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

    if (wrote)
        screen->dirty = true;
}

void draw_rect(Screen *screen, int x, int y, int w, int h, Uint32 color)
{
    draw_line(screen, x, y, x + w, y, color);
    draw_line(screen, x, y, x, y + h, color);
    draw_line(screen, x + w, y, x + w, y + h, color);
    draw_line(screen, x, y + h, x + w, y + h, color);
}

void draw_fillRect(Screen *screen, int x, int y, int w, int h, Uint32 color)
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
void draw_circle(Screen *screen, int x0, int y0, int radius, Uint32 color)
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

void draw_fillCircle(Screen *screen, int x0, int y0, int radius, Uint32 color)
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
void draw_polygon(Screen *screen, list_t *points, Uint32 color)
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

void draw_fillPolygon(Screen *screen, list_t *points, Uint32 color)
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

void screen_print(Screen *screen, const char *text, int x, int y, Uint32 color)
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
            screen->cursor_x += 4;
    }
}

// screen.c
int get_colorIndex(Uint32 pixel_color)
{
    Uint8 r = (pixel_color >> 16) & 0xFF;
    Uint8 g = (pixel_color >> 8) & 0xFF;
    Uint8 b = pixel_color & 0xFF;

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
