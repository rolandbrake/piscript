#include "pi_img.h"

#include "../common.h"

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct
{
    int width;
    int height;
    uint8_t *pixels;
    uint8_t *alpha;
    bool has_alpha;
    bool is_sprite;
} ImageSource;

/**
 * Retrieves an ImageSource object from the given value.
 *
 * This function retrieves an ImageSource object from the given value. If the value
 * is an image, then the ImageSource object is filled with the image's data.
 * If the value is a sprite, then the ImageSource object is filled with the sprite's
 * data. The alpha data of the ImageSource object is set to 0 for pixels with
 * value 0, and 1 for pixels with value 255.
 *
 * @param vm the virtual machine instance.
 * @param value the value to retrieve the ImageSource object from.
 * @param fn_name the name of the function that called this function.
 * @return the ImageSource object retrieved from the given value.
 */
static ImageSource get_imageSource(vm_t *vm, Value value, const char *fn_name)
{
    ImageSource src = {0, 0, NULL, NULL, false, false};

    /*
     * If the value is an image, then fill the ImageSource object with the image's data.
     */
    if (IS_IMAGE(value))
    {
        ObjImage *img = AS_IMAGE(value);
        src.width = img->width;
        src.height = img->height;
        src.pixels = img->pixels;
        src.alpha = img->alpha;
        return src;
    }

    /*
     * If the value is a sprite, then fill the ImageSource object with the sprite's data.
     */
    if (IS_SPRITE(value))
    {
        ObjSprite *sprite = AS_SPRITE(value);
        int size = (int)sprite->width * (int)sprite->height;
        uint8_t *alpha = malloc(size);
        if (!alpha)
            vm_errorf(vm, "[%s] memory allocation failed.", fn_name);

        for (int i = 0; i < size; i++)
            alpha[i] = (sprite->data[i] == 0) ? 0 : 1;

        src.width = (int)sprite->width;
        src.height = (int)sprite->height;
        src.pixels = sprite->data;
        src.alpha = alpha;
        src.has_alpha = true;
        src.is_sprite = true;
        return src;
    }

    /*
     * If the value is neither an image nor a sprite, then raise an error.
     */
    vm_errorf(vm, "[%s] expects image or sprite as first argument.", fn_name);
    return src;
}

/**
 * Frees the resources associated with an ImageSource object.
 *
 * This function frees the pixels and alpha data associated with an ImageSource
 * object. If the object owns its alpha data, then it is freed as well.
 *
 * @param src the ImageSource object to free.
 */
static void free_imageSource(ImageSource *src)
{
    if (src->has_alpha && src->alpha)
    {
        // Free the alpha data if the object owns it
        free(src->alpha);
    }
}

/**
 * Creates an image result value from the given image source data.
 *
 * If the source is not a sprite, then a palette-based image object is created.
 * Otherwise, a sprite object is created with the given width, height, and data.
 *
 * @param vm The virtual machine instance.
 * @param src The image source data (pixels, alpha, width, height).
 * @param w The width of the image result.
 * @param h The height of the image result.
 * @param pixels The pixel data of the image result.
 * @param alpha The alpha channel data of the image result.
 * @param fn_name The name of the function calling this function.
 * @return An image result value (ObjImage or ObjSprite).
 */
static Value make_imageResult(vm_t *vm, const ImageSource *src, int w, int h, uint8_t *pixels, uint8_t *alpha, const char *fn_name)
{
    if (!src->is_sprite)
        return NEW_OBJ(new_image(w, h, pixels, alpha));

    if (w > UINT16_MAX || h > UINT16_MAX)
        vm_errorf(vm, "[%s] sprite result exceeds max size 65535x65535.", fn_name);

    int size = w * h;
    uint8_t *data = malloc(size);
    if (!data)
        vm_errorf(vm, "[%s] memory allocation failed.", fn_name);

    // Copy the pixel data to the sprite data, replacing transparent pixels with 0
    for (int i = 0; i < size; i++)
        data[i] = (alpha[i] == 0) ? 0 : pixels[i];

    // Free the temporary pixel and alpha data
    free(pixels);
    free(alpha);

    // Create and return a sprite object with the given width, height, and data
    return NEW_OBJ(new_sprite((uint16_t)w, (uint16_t)h, data));
}

/**
 * Loads an image from a file and converts it to a palette-based image object.
 *
 * This function expects a single argument: the file path to the image.
 * It reads the image, converts it to a 32-bit RGBA format, and maps it
 * to a palette-based representation with alpha transparency.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1).
 * @param argv The arguments: file path (string).
 * @return A new image object.
 */
Value pi_image(vm_t *vm, int argc, Value *argv)
{
    // Validate the input argument
    if (argc < 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[image] expects a file path string as its first argument.");

    const char *path = AS_CSTRING(argv[0]);

    // Load the image using SDL_image library
    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
        vm_errorf(vm, "[image] failed to load: %s", IMG_GetError());

    // Convert the loaded surface to 32-bit RGBA format
    SDL_Surface *formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface); // Free the original surface

    if (!formatted)
        vm_error(vm, "[image] failed to convert image to RGBA32 format.");

    int w = formatted->w;
    int h = formatted->h;

    // Allocate memory for pixel data and alpha channel
    uint8_t *pixels = malloc(w * h);
    uint8_t *alpha = malloc(w * h);
    if (!pixels || !alpha)
    {
        SDL_FreeSurface(formatted);
        vm_error(vm, "[image] memory allocation failed.");
    }

    // Access the pixel data from the formatted surface
    Uint32 *source = (Uint32 *)formatted->pixels;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            Uint32 pixel = source[y * w + x];
            uint8_t r, g, b, a;
            SDL_GetRGBA(pixel, formatted->format, &r, &g, &b, &a);
            // Map the pixel color to the closest palette index and store alpha
            pixels[y * w + x] = find_paletteColor(r, g, b);
            alpha[y * w + x] = a / 255.0f; // Normalize alpha to 0-1 range
        }
    }

    SDL_FreeSurface(formatted);

    // Create a new image object from the pixel data
    ObjImage *img = new_image(w, h, pixels, alpha);
    return NEW_OBJ(img);
}

/**
 * Crops a specific region from an image and returns it as a new image.
 *
 * This function expects five arguments: the image to be cropped, the x and y
 * coordinates of the top-left corner of the crop region, and the width and
 * height of the crop region. The width and height must be positive.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (5).
 * @param argv The arguments: image, x, y, width, and height.
 * @return A new image object representing the cropped region.
 */
Value pi_crop(vm_t *vm, int argc, Value *argv)
{
    if (argc < 5)
        vm_error(vm, "[crop] expects (image|sprite, x, y, width, height)");

    ImageSource src = get_imageSource(vm, argv[0], "crop");
    int x = AS_INT(argv[1]);
    int y = AS_INT(argv[2]);
    int w = AS_INT(argv[3]);
    int h = AS_INT(argv[4]);

    if (w <= 0 || h <= 0)
        vm_error(vm, "[crop] width and height must be positive");

    // Allocate memory for the new cropped image data
    uint8_t *pixels = malloc(w * h);
    uint8_t *alpha = malloc(w * h);
    if (!pixels || !alpha)
        vm_error(vm, "[crop] memory allocation failed");

    // Copy pixels from the source image to the new cropped image
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int src_x = x + i;
            int src_y = y + j;
            int dist_index = j * w + i;

            // Check bounds and copy pixel data if within source image bounds
            if (src_x >= 0 && src_x < src.width &&
                src_y >= 0 && src_y < src.height)
            {
                int src_index = src_y * src.width + src_x;
                pixels[dist_index] = src.pixels[src_index];
                alpha[dist_index] = src.alpha[src_index];
            }
            else
            {
                // Fill out-of-bounds areas with default color and transparency
                pixels[dist_index] = 0; // Default color
                alpha[dist_index] = 0;  // Transparent
            }
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, w, h, pixels, alpha, "crop");
}

/**
 * Resizes an image to a new size using nearest-neighbor interpolation.
 *
 * This function expects three arguments: the image to be resized, and the new
 * width and height of the image. The width and height must be positive.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (3).
 * @param argv The arguments: image, new_width, and new_height.
 * @return A new image object that is the result of resizing the original
 *   image.
 */
Value pi_resize(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
        vm_error(vm, "[resize] expects (image|sprite, new_width, new_height)");

    ImageSource src = get_imageSource(vm, argv[0], "resize");
    int new_w = AS_INT(argv[1]);
    int new_h = AS_INT(argv[2]);

    if (new_w <= 0 || new_h <= 0)
        vm_error(vm, "[resize] width and height must be positive");

    uint8_t *new_pixels = malloc(new_w * new_h);
    uint8_t *new_alpha = malloc(new_w * new_h);

    if (!new_pixels || !new_alpha)
        vm_error(vm, "[resize] memory allocation failed");

    // Nearest-neighbor resizing
    for (int j = 0; j < new_h; j++)
    {
        for (int i = 0; i < new_w; i++)
        {
            int src_x = i * src.width / new_w;
            int src_y = j * src.height / new_h;
            int src_index = src_y * src.width + src_x;
            int dist_index = j * new_w + i;

            new_pixels[dist_index] = src.pixels[src_index];
            new_alpha[dist_index] = src.alpha[src_index];
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, new_w, new_h, new_pixels, new_alpha, "resize");
}

/**
 * Displays an image on the screen at the specified position.
 *
 * This function expects at least one argument: the image to be displayed. If
 * two or three additional arguments are provided, they are interpreted as the
 * x and y coordinates (respectively) of the top-left corner of the image on
 * the screen. If omitted, the image is drawn at the current cursor position.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1 to 3).
 * @param argv The arguments: image (2D image), and optionally x and y coordinates.
 * @return A nil value indicating completion.
 */
Value pi_rend2d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[show] expects (image|sprite [, x, y])");

    ImageSource img = get_imageSource(vm, argv[0], "show");
    int dx = (argc > 1 && IS_NUM(argv[1])) ? AS_INT(argv[1]) : 0;
    int dy = (argc > 2 && IS_NUM(argv[2])) ? AS_INT(argv[2]) : 0;

    // Iterate over the image pixels
    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            int screen_x = dx + x;
            int screen_y = dy + y;

            // Skip if outside the screen
            if (screen_x < 0 || screen_x >= 128 ||
                screen_y < 0 || screen_y >= 128)
                continue;

            int index = y * img.width + x;
            uint8_t color = img.pixels[index];
            uint8_t alpha = img.alpha[index];

            // Skip transparent pixels
            if (alpha < 0.01f)
                continue;

            // Set the pixel color and alpha
            set_pixel_alpha(vm->screen, screen_x, screen_y, color, alpha);
        }
    }

    free_imageSource(&img);
    return NEW_NIL();
}

/**
 * Scales an image by a factor in both x and y directions.
 *
 * This function expects three arguments: the image to be scaled, the scale
 * factor in the x direction, and the scale factor in the y direction. The
 * scale factors can be any positive number, and negative values are not
 * allowed. If the resulting image size is zero (i.e., the scale factors are
 * too small), an error is raised.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects 3).
 * @param argv The arguments: the image, sx, and sy.
 * @return A new image object that is the result of scaling the original
 *   image by the given scale factors.
 */
Value pi_scale2d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
        vm_error(vm, "[scale2d] expects (image|sprite, sx, sy)");

    ImageSource src = get_imageSource(vm, argv[0], "scale2d");
    double sx = AS_NUM(argv[1]);
    double sy = AS_NUM(argv[2]);

    if (sx <= 0 || sy <= 0)
        vm_error(vm, "[scale2d] scale factors must be > 0");

    int new_w = (int)(src.width * sx);
    int new_h = (int)(src.height * sy);

    if (new_w == 0 || new_h == 0)
        vm_error(vm, "[scale2d] resulting image size is zero");

    uint8_t *new_pixels = malloc(new_w * new_h);
    uint8_t *new_alpha = malloc(new_w * new_h);

    if (!new_pixels || !new_alpha)
        vm_error(vm, "[scale2d] memory allocation failed");

    // Iterate over each pixel in the new image and map it to the original
    // image using the scale factors. Nearest-neighbor interpolation is used.
    for (int y = 0; y < new_h; y++)
    {
        for (int x = 0; x < new_w; x++)
        {
            // Map the new coordinates to the original image
            int src_x = (int)(x / sx);
            int src_y = (int)(y / sy);

            // Handle out-of-bounds cases
            if (src_x >= src.width)
                src_x = src.width - 1;
            if (src_y >= src.height)
                src_y = src.height - 1;

            int src_index = src_y * src.width + src_x;
            int dst_index = y * new_w + x;

            // Copy the pixel and alpha values from the original image
            new_pixels[dst_index] = src.pixels[src_index];
            new_alpha[dst_index] = src.alpha[src_index];
        }
    }

    // Create a new image object with the scaled image data
    free_imageSource(&src);
    return make_imageResult(vm, &src, new_w, new_h, new_pixels, new_alpha, "scale2d");
}

/**
 * Translate an image by a given amount of pixels in both x and y direction.
 *
 * The image is translated by the given amounts of pixels in the x and y
 * direction. This function expects three arguments: the image to be
 * translated and the amounts of pixels to translate in the x and y
 * directions. Negative values are allowed for the translation amounts.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects 3).
 * @param argv The arguments: the image, dx, and dy.
 * @return A new image object that is the result of translating the original
 *   image by the given amounts of pixels.
 */
Value pi_tran2d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3)
        vm_error(vm, "[tran2d] expects (image|sprite, dx, dy)");

    ImageSource src = get_imageSource(vm, argv[0], "tran2d");
    int dx = AS_INT(argv[1]);
    int dy = AS_INT(argv[2]);

    int w = src.width, h = src.height;
    uint8_t *new_pixels = malloc(w * h);
    uint8_t *new_alpha = malloc(w * h);
    if (!new_pixels || !new_alpha)
        vm_error(vm, "[tran2d] memory allocation failed");

    // Fill transparent by default
    memset(new_pixels, 0, w * h);
    memset(new_alpha, 0, w * h);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < w && ny >= 0 && ny < h)
            {
                int src_idx = y * w + x;
                int dst_idx = ny * w + nx;
                new_pixels[dst_idx] = src.pixels[src_idx];
                new_alpha[dst_idx] = src.alpha[src_idx];
            }
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, w, h, new_pixels, new_alpha, "tran2d");
}

/**
 * Flips the image horizontally and/or vertically based on the given flags.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects 2 or 3).
 * @param argv The arguments: image, flip_x, and optionally flip_y.
 * @return A new flipped image object.
 */
Value pi_flip(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        vm_error(vm, "[flip] expects (image|sprite, flip_x [, flip_y])");

    ImageSource src = get_imageSource(vm, argv[0], "flip");
    bool flip_x = AS_BOOL(argv[1]);
    bool flip_y = (argc > 2 && IS_BOOL(argv[2])) ? AS_BOOL(argv[2]) : false;

    int w = src.width, h = src.height;
    uint8_t *new_pixels = malloc(w * h);
    uint8_t *new_alpha = malloc(w * h);
    if (!new_pixels || !new_alpha)
        vm_error(vm, "[flip] memory allocation failed");

    // Iterate over each pixel and determine new position based on flip flags
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int src_x = flip_x ? (w - 1 - x) : x;
            int src_y = flip_y ? (h - 1 - y) : y;
            int src_idx = src_y * w + src_x;
            int dst_idx = y * w + x;

            // Copy pixel and alpha values to the new position
            new_pixels[dst_idx] = src.pixels[src_idx];
            new_alpha[dst_idx] = src.alpha[src_idx];
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, w, h, new_pixels, new_alpha, "flip");
}

/**
 * Rotate an image by a given angle in degrees. The image is rotated by
 * translating its pixels to the origin, applying the rotation, and then
 * translating them back to their original positions. This means that the
 * output image may be slightly larger than the input image, as the rotation
 * can make the image "grow" in size.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (expects 2 or 3).
 * @param argv The arguments: image, angle_degrees, and optionally width and
 *             height of the output image.
 * @return A new rotated image object.
 */
Value pi_rotate2d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2 || !IS_NUM(argv[1]))
        vm_error(vm, "[rot2d] expects (image|sprite, angle_degrees)");

    ImageSource src = get_imageSource(vm, argv[0], "rot2d");
    double angle_deg = AS_NUM(argv[1]);
    double angle_rad = angle_deg * M_PI / 180.0;

    int w = src.width;
    int h = src.height;

    // Rotate around the true image center; this avoids half-pixel drift on even sizes.
    double cx = ((double)w - 1.0) * 0.5;
    double cy = ((double)h - 1.0) * 0.5;

    // Output image will be same size (can be adjusted later to auto-expand)
    int new_w = w;
    int new_h = h;

    uint8_t *new_pixels = malloc(new_w * new_h);
    uint8_t *new_alpha = malloc(new_w * new_h);
    if (!new_pixels || !new_alpha)
        vm_error(vm, "[rot2d] memory allocation failed");

    memset(new_pixels, 0, new_w * new_h);
    memset(new_alpha, 0, new_w * new_h);

    double cos_theta = cos(-angle_rad); // negative for backward mapping
    double sin_theta = sin(-angle_rad);

    for (int y = 0; y < new_h; y++)
    {
        for (int x = 0; x < new_w; x++)
        {
            // Translate to origin
            double dx = x - cx;
            double dy = y - cy;

            // Rotate backward
            double src_x = dx * cos_theta - dy * sin_theta + cx;
            double src_y = dx * sin_theta + dy * cos_theta + cy;

            int sx = (int)lround(src_x);
            int sy = (int)lround(src_y);

            int dst_idx = y * new_w + x;

            if (sx >= 0 && sx < w && sy >= 0 && sy < h)
            {
                int src_idx = sy * w + sx;
                new_pixels[dst_idx] = src.pixels[src_idx];
                new_alpha[dst_idx] = src.alpha[src_idx];
            }
            else
            {
                new_pixels[dst_idx] = 0;
                new_alpha[dst_idx] = 0;
            }
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, new_w, new_h, new_pixels, new_alpha, "rot2d");
}

/**
 * @brief Create a deep copy of an image object.
 *
 * @param vm The virtual machine instance.
 * @param argc Number of arguments (should be 1).
 * @param argv Arguments: [image]
 * @return A new copy of the image object.
 */
Value pi_copy2d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1)
        vm_error(vm, "[copy2d] expects (image|sprite)");

    ImageSource src = get_imageSource(vm, argv[0], "copy2d");
    int size = src.width * src.height;

    uint8_t *pixels = malloc(size);
    uint8_t *alpha = malloc(size);

    if (!pixels || !alpha)
        vm_error(vm, "[copy2d] memory allocation failed");

    // Perform a deep copy of the image data
    memcpy(pixels, src.pixels, size);
    memcpy(alpha, src.alpha, size);

    free_imageSource(&src);
    return make_imageResult(vm, &src, src.width, src.height, pixels, alpha, "copy2d");
}
