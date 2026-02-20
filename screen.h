/* screen.h - Header file for handling the screen display and rendering */

#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "list.h"

// Screen dimensions
#define SCREEN_WIDTH 128  // Width of the screen in pixels
#define SCREEN_HEIGHT 128 // Height of the screen in pixels
#define SCALE 4           // Scale factor for rendering (affects window size)
#define PALETTE_SIZE 32   // Number of colors in the palette
#define NUM_COLORS 32     // Maximum number of colors supported

typedef enum
{
    COLOR_BLACK = 0,
    COLOR_DARK_BLUE,
    COLOR_DARK_MAGENTA,
    COLOR_DARK_GREEN,
    COLOR_BROWN,
    COLOR_DARK_GRAY,
    // COLOR_LIGHT_GRAY, // skipped in palette
    COLOR_WHITE,
    COLOR_VERY_LIGHT_PINK,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_ORANGE,
    COLOR_BRIGHT_YELLOW,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_BLUE,
    COLOR_SOFT_PURPLE,
    COLOR_BRIGHT_PINK,
    COLOR_PEACH,
    COLOR_DARK_BROWN,
    COLOR_NAVY_BLUE,
    COLOR_DEEP_PURPLE,
    COLOR_TEAL,
    COLOR_RUST_RED,
    COLOR_MUTED_PURPLE,
    COLOR_WARM_GRAY,
    COLOR_PALE_LIME,
    COLOR_DARK_PINK,
    COLOR_ORANGE_RED,
    COLOR_LIME_GREEN,
    COLOR_EMERALD_GREEN,
    COLOR_COBALT_BLUE,
    COLOR_DUSKY_PURPLE,
    COLOR_CORAL,
    COLOR_LIGHT_SALMON,

    COLOR_COUNT = PALETTE_SIZE
} Color;

#define CHAR_WIDTH 4  // Width of a character in pixels
#define CHAR_HEIGHT 6 // Height of a character in pixels

#define MAX_SPRITES 256

typedef struct
{
    uint16_t width;  // Width of the sprite in pixels
    uint16_t height; // Height of the sprite in pixels
    uint8_t *pixels; // Allocated memory holding pixel data
} Sprite;

// Structure representing the screen and its properties
typedef struct
{
    SDL_Window *window;     // Pointer to the SDL window
    SDL_Renderer *renderer; // Pointer to the SDL renderer
    SDL_Texture *texture;   // Texture used for pixel rendering
    Uint32 *pixels;         // Array storing pixel colors
    int cursor_x;           // Current x position of the text cursor
    int cursor_y;           // Current y position of the text cursor
    Color text_color;       // Current text color

} Screen;

// Initializes the screen and SDL components
// Returns a pointer to the newly created Screen instance
Screen *screen_init(Color color);

// Closes the screen and releases allocated resources
void screen_close(Screen *screen);

// Updates the display by refreshing the texture and rendering
void screen_update(Screen *screen);

// Clears the screen with a specific color
void screen_clear(Screen *screen, Color color);

// Sets a single pixel at (x, y) to a specified color
void set_pixel(Screen *screen, int x, int y, Color color);

// Draws a single pixel at (x, y) with a specified color and specified alpha
void set_pixel_alpha(Screen *screen, int x, int y, Color color, double alpha);

// Draws a single pixel at (x, y) with a specified color and specified shade/brightness
void set_pixel_shaded(Screen *screen, int x, int y, Color color, float brightness);

// Draws a line from (x0, y0) to (x1, y1) with a specified color
void draw_line(Screen *screen, int x0, int y0, int x1, int y1, Color color);

// Draws an unfilled rectangle with top-left corner (x, y), width w, height h
void draw_rect(Screen *screen, int x, int y, int w, int h, Color color);

// Draws a filled rectangle with top-left corner (x, y), width w, height h
void draw_fillRect(Screen *screen, int x, int y, int w, int h, Color color);

// Draws an unfilled circle with center (x0, y0) and a given radius
void draw_circle(Screen *screen, int x0, int y0, int radius, Color color);

// Draws a filled circle with center (x0, y0) and a given radius
void draw_fillCircle(Screen *screen, int x0, int y0, int radius, Color color);

// Draws an unfilled polygon using a list of points
void draw_polygon(Screen *screen, list_t *points, Color color);

// Draws a filled polygon using a list of points
void draw_fillPolygon(Screen *screen, list_t *points, Color color);

// Renders text on the screen at position (x, y) with a specified color
void screen_print(Screen *screen, const char *text, int x, int y, Color color);

int get_colorIndex(Uint32 pixel_color);

void draw_matrix(Screen *screen, int x, int y, int w, int h, uint8_t matrix[h][w]);

#endif
