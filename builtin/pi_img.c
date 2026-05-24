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
    Uint32 *pixels;
    uint8_t *alpha;
    bool has_alpha;
    bool is_sprite;
} ImageSource;

static Uint32 pack_argb(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return ((Uint32)a << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
}

static uint8_t image_alphaAt(const ImageSource *src, int index)
{
    if (src->alpha)
        return src->alpha[index];
    return (uint8_t)((src->pixels[index] >> 24) & 0xff);
}

static Uint32 sprite_color(uint8_t index)
{
    Uint32 color = 0;
    if (!screen_paletteColor(index, &color))
        screen_paletteColor(0, &color);
    return color;
}

static uint8_t clamp_u8(double value)
{
    if (value < 0.0)
        return 0;
    if (value > 255.0)
        return 255;
    return (uint8_t)round(value);
}

static double clamp_unit(double value)
{
    if (value < 0.0)
        return 0.0;
    if (value > 1.0)
        return 1.0;
    return value;
}

static Uint32 image_colorArg(vm_t *vm, Value value, const char *fn_name)
{
    Uint32 color = 0;
    if (!IS_NUM(value) || !screen_colorFromNumber(AS_NUM(value), &color))
        vm_errorf(vm, "[%s] color must be a palette index or packed 0xAARRGGBB number.", fn_name);

    return screen_resolveColor(color);
}

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
        return src;
    }

    /*
     * If the value is a sprite, then fill the ImageSource object with the sprite's data.
     */
    if (IS_SPRITE(value))
    {
        ObjSprite *sprite = AS_SPRITE(value);
        int size = (int)sprite->width * (int)sprite->height;
        Uint32 *pixels = malloc((size_t)size * sizeof(Uint32));
        uint8_t *alpha = malloc(size);
        if (!pixels || !alpha)
            vm_errorf(vm, "[%s] memory allocation failed.", fn_name);

        for (int i = 0; i < size; i++)
        {
            pixels[i] = sprite_color(sprite->pixels[i]);
            alpha[i] = (sprite->pixels[i] == 0) ? 0 : 255;
        }

        src.width = (int)sprite->width;
        src.height = (int)sprite->height;
        src.pixels = pixels;
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
    if (src->has_alpha && src->pixels)
        free(src->pixels);
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
static Value make_imageResult(vm_t *vm, const ImageSource *src, int w, int h, Uint32 *pixels, uint8_t *alpha, const char *fn_name)
{
    if (!src->is_sprite)
    {
        if (alpha)
        {
            int size = w * h;
            for (int i = 0; i < size; i++)
                pixels[i] = (pixels[i] & 0x00ffffffu) | ((Uint32)alpha[i] << 24);
            free(alpha);
        }
        return NEW_OBJ(new_image(w, h, pixels));
    }

    if (w > UINT16_MAX || h > UINT16_MAX)
        vm_errorf(vm, "[%s] sprite result exceeds max size 65535x65535.", fn_name);

    int size = w * h;
    uint8_t *data = malloc(size);
    if (!data)
        vm_errorf(vm, "[%s] memory allocation failed.", fn_name);

    // Copy the pixel data to the sprite data, replacing transparent pixels with 0
    for (int i = 0; i < size; i++)
        data[i] = (alpha[i] == 0) ? 0 : (uint8_t)get_colorIndex(pixels[i]);

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
static ObjImage *load_image(vm_t *vm, const char *path)
{
    char *resolved = resolve_sourcePath(vm->source_path, path);
    if (!resolved)
        vm_error(vm, "[image] memory allocation failed while resolving the path.");

    SDL_Surface *surface = IMG_Load(resolved);
    if (!surface)
    {
        free(resolved);
        vm_errorf(vm, "[image] failed to load '%s': %s", path, IMG_GetError());
    }
    free(resolved);

    // Convert the loaded surface to 32-bit RGBA format
    SDL_Surface *formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface); // Free the original surface

    if (!formatted)
        vm_error(vm, "[image] failed to convert image to RGBA32 format.");

    int w = formatted->w;
    int h = formatted->h;

    // Allocate memory for packed ARGB pixel data.
    Uint32 *pixels = malloc((size_t)w * (size_t)h * sizeof(Uint32));
    if (!pixels)
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
            pixels[y * w + x] = pack_argb(r, g, b, a);
        }
    }

    SDL_FreeSurface(formatted);

    // Create a new image object from the pixel data
    return new_image(w, h, pixels);
}

Value pi_image(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_STRING(argv[0]))
        vm_error(vm, "[image] expects a file path string as its first argument.");

    return NEW_OBJ(load_image(vm, AS_CSTRING(argv[0])));
}

Value pi_spriteFile(vm_t *vm, const char *path)
{
    ObjImage *img = load_image(vm, path);
    if (img->width > UINT16_MAX || img->height > UINT16_MAX)
        vm_error(vm, "[sprite] image exceeds max sprite size 65535x65535.");

    int size = img->width * img->height;
    uint8_t *data = malloc(size);
    if (!data)
        vm_error(vm, "[sprite] memory allocation failed.");

    for (int i = 0; i < size; i++)
    {
        uint8_t alpha = (uint8_t)((img->pixels[i] >> 24) & 0xff);
        uint8_t r = (uint8_t)((img->pixels[i] >> 16) & 0xff);
        uint8_t g = (uint8_t)((img->pixels[i] >> 8) & 0xff);
        uint8_t b = (uint8_t)(img->pixels[i] & 0xff);
        data[i] = alpha == 0 ? 0 : (uint8_t)find_paletteColor(r, g, b);
    }

    uint16_t width = (uint16_t)img->width;
    uint16_t height = (uint16_t)img->height;
    free(img->pixels);
    free(img);
    return NEW_OBJ(new_sprite(width, height, data));
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
    Uint32 *pixels = malloc((size_t)w * (size_t)h * sizeof(Uint32));
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
                alpha[dist_index] = image_alphaAt(&src, src_index);
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
    if (argc < 1 || !IS_IMAGE(argv[0]))
        vm_error(vm, "[resize] expects image as first argument.");

    bool no_w = (!IS_NUM(argv[1]) || AS_INT(argv[1]) <= 0);
    bool no_h = (!IS_NUM(argv[2]) || AS_INT(argv[2]) <= 0);

    // At least one dimension must be provided
    if (no_w && no_h)
        vm_error(vm, "[resize] expect at least one of width or height to be a positive number.");

    ImageSource src = get_imageSource(vm, argv[0], "resize");

    int new_w;
    int new_h;

    // Preserve aspect ratio if one dimension is nil
    if (no_w)
    {
        new_h = AS_INT(argv[2]);

        if (new_h <= 0)
            vm_error(vm, "[resize] height must be positive");

        float aspect = (float)src.width / (float)src.height;
        new_w = (int)(new_h * aspect + 0.5f);
    }
    else if (no_h)
    {
        new_w = AS_INT(argv[1]);

        if (new_w <= 0)
            vm_error(vm, "[resize] width must be positive");

        float aspect = (float)src.height / (float)src.width;
        new_h = (int)(new_w * aspect + 0.5f);
    }
    else
    {
        new_w = AS_INT(argv[1]);
        new_h = AS_INT(argv[2]);
    }

    Uint32 *new_pixels = malloc((size_t)new_w * (size_t)new_h * sizeof(Uint32));
    uint8_t *new_alpha = malloc((size_t)new_w * (size_t)new_h);

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
            new_alpha[dist_index] = image_alphaAt(&src, src_index);
        }
    }

    free_imageSource(&src);

    return make_imageResult(
        vm,
        &src,
        new_w,
        new_h,
        new_pixels,
        new_alpha,
        "resize");
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
            int view_x = screen_x - vm->screen->offset_x;
            int view_y = screen_y - vm->screen->offset_y;

            // Skip if outside the screen
            if (view_x < 0 || view_x >= SCREEN_WIDTH ||
                view_y < 0 || view_y >= SCREEN_HEIGHT)
                continue;

            int index = y * img.width + x;
            Uint32 color = img.pixels[index];
            uint8_t alpha = image_alphaAt(&img, index);

            // Skip transparent pixels
            if (alpha == 0)
                continue;

            // Set the pixel color and alpha
            set_pixelAlpha(vm->screen, screen_x, screen_y, color, alpha / 255.0);
        }
    }

    free_imageSource(&img);
    return NEW_NIL();
}

Value pi_get2d(vm_t *vm, int argc, Value *argv)
{
    if (argc != 3)
        vm_error(vm, "[get2d] expects (image|sprite, x, y)");
    if (!IS_NUM(argv[1]) || !IS_NUM(argv[2]))
        vm_error(vm, "[get2d] x and y must be numeric.");

    ImageSource img = get_imageSource(vm, argv[0], "get2d");
    int x = AS_INT(argv[1]);
    int y = AS_INT(argv[2]);

    if (x < 0 || x >= img.width || y < 0 || y >= img.height)
        vm_error(vm, "[get2d] pixel coordinates out of bounds.");

    int index = y * img.width + x;
    Uint32 color = img.pixels[index];
    uint8_t alpha = image_alphaAt(&img, index);
    color = (color & 0x00ffffffu) | ((Uint32)alpha << 24);

    free_imageSource(&img);
    return NEW_NUM((double)color);
}

Value pi_set2d(vm_t *vm, int argc, Value *argv)
{
    if (argc != 4 && argc != 5)
        vm_error(vm, "[set2d] expects (image|sprite, x, y, color [, alpha])");
    if (!IS_NUM(argv[1]) || !IS_NUM(argv[2]))
        vm_error(vm, "[set2d] x and y must be numeric.");

    int x = AS_INT(argv[1]);
    int y = AS_INT(argv[2]);
    Uint32 color = 0;
    if (!IS_NUM(argv[3]) || !screen_colorFromNumber(AS_NUM(argv[3]), &color))
        vm_error(vm, "[set2d] color must be a palette index or packed 0xAARRGGBB number.");

    color = screen_resolveColor(color);

    if (argc == 5)
    {
        if (!IS_NUM(argv[4]))
            vm_error(vm, "[set2d] alpha must be numeric.");
        double alpha = AS_NUM(argv[4]);
        if (alpha < 0.0)
            alpha = 0.0;
        if (alpha > 1.0)
            alpha = 1.0;
        color = (color & 0x00ffffffu) | ((Uint32)round(alpha * 255.0) << 24);
    }

    if (IS_IMAGE(argv[0]))
    {
        ObjImage *img = AS_IMAGE(argv[0]);
        if (x < 0 || x >= img->width || y < 0 || y >= img->height)
            vm_error(vm, "[set2d] pixel coordinates out of bounds.");
        img->pixels[y * img->width + x] = color;
        return NEW_NIL();
    }

    if (IS_SPRITE(argv[0]))
    {
        ObjSprite *sprite = AS_SPRITE(argv[0]);
        if (x < 0 || x >= sprite->width || y < 0 || y >= sprite->height)
            vm_error(vm, "[set2d] pixel coordinates out of bounds.");

        uint8_t alpha = (uint8_t)((color >> 24) & 0xff);
        sprite->pixels[y * sprite->width + x] = alpha == 0 ? 0 : (uint8_t)get_colorIndex(color);
        return NEW_NIL();
    }

    vm_error(vm, "[set2d] expects image or sprite as first argument.");
    return NEW_NIL();
}

Value pi_show(vm_t *vm, int argc, Value *argv)
{
    if (argc != 1 && argc != 3 && argc != 4 && argc != 5 && argc != 6)
        vm_error(vm, "[show] expects image|sprite [, x, y [, width, height] [, centered]]");

    bool centered = false;
    if (argc == 4 || argc == 6)
    {
        if (!IS_BOOL(argv[argc - 1]))
            vm_error(vm, "[show] centered must be a boolean.");
        centered = AS_BOOL(argv[argc - 1]);
    }

    ImageSource img = get_imageSource(vm, argv[0], "show");
    int dx = 0;
    int dy = 0;
    int draw_w = img.width;
    int draw_h = img.height;

    if (argc >= 3)
    {
        if (!IS_NUM(argv[1]) || !IS_NUM(argv[2]))
            vm_error(vm, "[show] x and y must be numeric.");
        dx = AS_INT(argv[1]);
        dy = AS_INT(argv[2]);
    }

    if (argc >= 5)
    {
        if (!IS_NUM(argv[3]) || !IS_NUM(argv[4]))
            vm_error(vm, "[show] width and height must be numeric.");
        draw_w = AS_INT(argv[3]);
        draw_h = AS_INT(argv[4]);
        if (draw_w <= 0 || draw_h <= 0)
            vm_error(vm, "[show] width and height must be positive.");
    }

    if (centered)
    {
        dx -= draw_w / 2;
        dy -= draw_h / 2;
    }

    for (int y = 0; y < draw_h; y++)
    {
        int src_y = y * img.height / draw_h;
        for (int x = 0; x < draw_w; x++)
        {
            int screen_x = dx + x;
            int screen_y = dy + y;
            int view_x = screen_x - vm->screen->offset_x;
            int view_y = screen_y - vm->screen->offset_y;
            if (view_x < 0 || view_x >= SCREEN_WIDTH ||
                view_y < 0 || view_y >= SCREEN_HEIGHT)
                continue;

            int src_x = x * img.width / draw_w;
            int index = src_y * img.width + src_x;
            uint8_t alpha = image_alphaAt(&img, index);
            if (alpha == 0)
                continue;

            set_pixelAlpha(vm->screen, screen_x, screen_y, img.pixels[index], alpha / 255.0);
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

    Uint32 *new_pixels = malloc((size_t)new_w * (size_t)new_h * sizeof(Uint32));
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
            new_alpha[dst_index] = image_alphaAt(&src, src_index);
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
    Uint32 *new_pixels = malloc((size_t)w * (size_t)h * sizeof(Uint32));
    uint8_t *new_alpha = malloc(w * h);
    if (!new_pixels || !new_alpha)
        vm_error(vm, "[tran2d] memory allocation failed");

    // Fill transparent by default
    memset(new_pixels, 0, (size_t)w * (size_t)h * sizeof(Uint32));
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
                new_alpha[dst_idx] = image_alphaAt(&src, src_idx);
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
    Uint32 *new_pixels = malloc((size_t)w * (size_t)h * sizeof(Uint32));
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
            new_alpha[dst_idx] = image_alphaAt(&src, src_idx);
        }
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, w, h, new_pixels, new_alpha, "flip");
}

Value pi_tint(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        vm_error(vm, "[tint] expects (image|sprite, color [, amount])");

    ImageSource src = get_imageSource(vm, argv[0], "tint");
    Uint32 tint = image_colorArg(vm, argv[1], "tint");
    double amount = (argc >= 3 && IS_NUM(argv[2])) ? clamp_unit(AS_NUM(argv[2])) : 1.0;

    int size = src.width * src.height;
    Uint32 *pixels = malloc((size_t)size * sizeof(Uint32));
    uint8_t *alpha = malloc(size);
    if (!pixels || !alpha)
        vm_error(vm, "[tint] memory allocation failed");

    uint8_t tr = (uint8_t)((tint >> 16) & 0xff);
    uint8_t tg = (uint8_t)((tint >> 8) & 0xff);
    uint8_t tb = (uint8_t)(tint & 0xff);

    for (int i = 0; i < size; i++)
    {
        Uint32 color = src.pixels[i];
        uint8_t r = (uint8_t)((color >> 16) & 0xff);
        uint8_t g = (uint8_t)((color >> 8) & 0xff);
        uint8_t b = (uint8_t)(color & 0xff);

        uint8_t nr = clamp_u8((double)r * (1.0 - amount) + (double)tr * amount);
        uint8_t ng = clamp_u8((double)g * (1.0 - amount) + (double)tg * amount);
        uint8_t nb = clamp_u8((double)b * (1.0 - amount) + (double)tb * amount);
        alpha[i] = image_alphaAt(&src, i);
        pixels[i] = pack_argb(nr, ng, nb, alpha[i]);
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, src.width, src.height, pixels, alpha, "tint");
}

Value pi_mask(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2)
        vm_error(vm, "[mask] expects (image|sprite, color [, tolerance])");

    ImageSource src = get_imageSource(vm, argv[0], "mask");
    Uint32 mask = image_colorArg(vm, argv[1], "mask");
    int tolerance = (argc >= 3 && IS_NUM(argv[2])) ? AS_INT(argv[2]) : 0;
    if (tolerance < 0)
        tolerance = 0;

    int size = src.width * src.height;
    Uint32 *pixels = malloc((size_t)size * sizeof(Uint32));
    uint8_t *alpha = malloc(size);
    if (!pixels || !alpha)
        vm_error(vm, "[mask] memory allocation failed");

    int mr = (int)((mask >> 16) & 0xff);
    int mg = (int)((mask >> 8) & 0xff);
    int mb = (int)(mask & 0xff);

    for (int i = 0; i < size; i++)
    {
        Uint32 color = src.pixels[i];
        int r = (int)((color >> 16) & 0xff);
        int g = (int)((color >> 8) & 0xff);
        int b = (int)(color & 0xff);
        int diff = abs(r - mr) + abs(g - mg) + abs(b - mb);

        pixels[i] = color;
        alpha[i] = (diff <= tolerance) ? 0 : image_alphaAt(&src, i);
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, src.width, src.height, pixels, alpha, "mask");
}

Value pi_alpha(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2 || !IS_NUM(argv[1]))
        vm_error(vm, "[alpha] expects (image|sprite, amount)");

    ImageSource src = get_imageSource(vm, argv[0], "alpha");
    double amount = clamp_unit(AS_NUM(argv[1]));

    int size = src.width * src.height;
    Uint32 *pixels = malloc((size_t)size * sizeof(Uint32));
    uint8_t *alpha = malloc(size);
    if (!pixels || !alpha)
        vm_error(vm, "[alpha] memory allocation failed");

    for (int i = 0; i < size; i++)
    {
        pixels[i] = src.pixels[i];
        alpha[i] = clamp_u8((double)image_alphaAt(&src, i) * amount);
    }

    free_imageSource(&src);
    return make_imageResult(vm, &src, src.width, src.height, pixels, alpha, "alpha");
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

    Uint32 *new_pixels = malloc((size_t)new_w * (size_t)new_h * sizeof(Uint32));
    uint8_t *new_alpha = malloc(new_w * new_h);
    if (!new_pixels || !new_alpha)
        vm_error(vm, "[rot2d] memory allocation failed");

    memset(new_pixels, 0, (size_t)new_w * (size_t)new_h * sizeof(Uint32));
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
                new_alpha[dst_idx] = image_alphaAt(&src, src_idx);
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

    Uint32 *pixels = malloc((size_t)size * sizeof(Uint32));
    uint8_t *alpha = malloc(size);

    if (!pixels || !alpha)
        vm_error(vm, "[copy2d] memory allocation failed");

    // Perform a deep copy of the image data
    memcpy(pixels, src.pixels, (size_t)size * sizeof(Uint32));
    for (int i = 0; i < size; i++)
        alpha[i] = image_alphaAt(&src, i);

    free_imageSource(&src);
    return make_imageResult(vm, &src, src.width, src.height, pixels, alpha, "copy2d");
}
