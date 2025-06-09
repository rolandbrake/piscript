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

#define MAX_SPRITES 256

typedef struct
{
    uint16_t width;
    uint16_t height;
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
    int text_color;         // Current text color
    Sprite sprites[MAX_SPRITES];
    int sprite_count;
} Screen;

// Initializes the screen and SDL components
// Returns a pointer to the newly created Screen instance
Screen *screen_init();

// Closes the screen and releases allocated resources
void screen_close(Screen *screen);

// Updates the display by refreshing the texture and rendering
void screen_update(Screen *screen);

// Clears the screen with a specific color
void screen_clear(Screen *screen, int color);

// Sets a single pixel at (x, y) to a specified color
void set_pixel(Screen *screen, int x, int y, int color);

// Draws a single pixel at (x, y) with a specified color and specified alpha
void set_pixel_alpha(Screen *screen, int x, int y, int color, double alpha);

// Draws a line from (x0, y0) to (x1, y1) with a specified color
void draw_line(Screen *screen, int x0, int y0, int x1, int y1, int color);

// Draws an unfilled rectangle with top-left corner (x, y), width w, height h
void draw_rect(Screen *screen, int x, int y, int w, int h, int color);

// Draws a filled rectangle with top-left corner (x, y), width w, height h
void draw_fillRect(Screen *screen, int x, int y, int w, int h, int color);

// Draws an unfilled circle with center (x0, y0) and a given radius
void draw_circle(Screen *screen, int x0, int y0, int radius, int color);

// Draws a filled circle with center (x0, y0) and a given radius
void draw_fillCircle(Screen *screen, int x0, int y0, int radius, int color);

// Draws an unfilled polygon using a list of points
void draw_polygon(Screen *screen, list_t *points, int color);

// Draws a filled polygon using a list of points
void draw_fillPolygon(Screen *screen, list_t *points, int color);

// Renders text on the screen at position (x, y) with a specified color
void screen_print(Screen *screen, const char *text, int x, int y, int color);

int get_colorIndex(Uint32 pixel_color);

#endif
