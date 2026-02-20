#include "cart.h"

// Helper to prevent reading past the end of the file
#define FREAD_CHECK(ptr, size, count, stream)                                   \
    do                                                                          \
    {                                                                           \
        if (fread(ptr, size, count, stream) != count)                           \
        {                                                                       \
            perror("Failed to read from cartridge file, or file is corrupted"); \
            return NULL;                                                        \
        }                                                                       \
    } while (0)

Cart *cart_load(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Could not open cartridge file");
        return NULL;
    }

    Cart *cart = (Cart *)calloc(1, sizeof(Cart));
    if (!cart)
    {
        perror("Failed to allocate memory for cart");
        fclose(file);
        return NULL;
    }

    // Read and validate header
    FREAD_CHECK(cart, 1, sizeof(Cart) - sizeof(Sprite *) - sizeof(Sound *) - sizeof(uint8_t *), file);

    if (strncmp(cart->magic, CART_MAGIC, 3) != 0)
    {
        fprintf(stderr, "Invalid cartridge magic number.\\n");
        free(cart);
        fclose(file);
        return NULL;
    }

    // Load sprites
    if (cart->spr_count > 0)
    {
        cart->sprites = (Sprite *)calloc(cart->spr_count, sizeof(Sprite));
        if (!cart->sprites)
            goto CLEANUP_FAIL;

        for (int i = 0; i < cart->spr_count; i++)
        {
            FREAD_CHECK(&cart->sprites[i].width, sizeof(uint16_t), 1, file);
            FREAD_CHECK(&cart->sprites[i].height, sizeof(uint16_t), 1, file);
            size_t pixels_size = cart->sprites[i].width * cart->sprites[i].height;
            cart->sprites[i].pixels = (uint8_t *)malloc(pixels_size);
            if (!cart->sprites[i].pixels)
                goto CLEANUP_FAIL;

            FREAD_CHECK(cart->sprites[i].pixels, 1, pixels_size, file);
        }
    }

    // Load sounds
    if (cart->sfx_count > 0)
    {
        cart->sounds = (Sound *)calloc(cart->sfx_count, sizeof(Sound));
        if (!cart->sounds)
            goto CLEANUP_FAIL;
        for (int i = 0; i < cart->sfx_count; i++)
        {
            FREAD_CHECK(&cart->sounds[i].speed, sizeof(uint16_t), 1, file);
            FREAD_CHECK(&cart->sounds[i].length, sizeof(uint16_t), 1, file);
            // cart->sounds[i].length = NOTE_COUNT;
            FREAD_CHECK(cart->sounds[i].notes, sizeof(Note), NOTE_COUNT, file);
        }
    }

    // Load code
    if (cart->code_size > 0)
    {
        cart->code = (uint8_t *)malloc(cart->code_size);
        if (!cart->code)
            goto CLEANUP_FAIL;

        FREAD_CHECK(cart->code, 1, cart->code_size, file);
    }
    else
    {
        cart->code = (uint8_t *)calloc(1, 1);
        if (!cart->code)
            goto CLEANUP_FAIL;
    }

    fclose(file);
    return cart;

CLEANUP_FAIL:
    perror("Failed to allocate memory during cart loading");
    cart_free(cart);
    fclose(file);
    return NULL;
}

void cart_free(Cart *cart)
{
    if (!cart)
        return;

    // Free sprites
    if (cart->sprites)
    {
        for (int i = 0; i < cart->spr_count; i++)
            if (cart->sprites[i].pixels)
                free(cart->sprites[i].pixels);

        free(cart->sprites);
    }

    // Free sounds
    if (cart->sounds)
        free(cart->sounds);

    // Free code
    if (cart->code)
        free(cart->code);

    free(cart);
}
