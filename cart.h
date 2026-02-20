#ifndef CART_H
#define CART_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "audio.h"

#define CART_MAGIC "PX1"

typedef struct
{
    char magic[3];      // "PX1"
    uint16_t version;   // = 1
    uint16_t flags;     // reserved for future use
    uint16_t spr_count; // number of sprites
    uint16_t sfx_count; // number of sounds
    uint32_t code_size; // size of program code in bytes

    Sprite *sprites; // pointer to array of sprites
    Sound *sounds;   // pointer to array of sounds

    uint8_t *code; // pointer to program code
} Cart;

/**
 * @brief Loads a cartridge from a file.
 *
 * This function reads a cartridge file, validates its format, and loads the
 * contained sprites, sounds, and code into memory.
 *
 * @param filename The path to the cartridge file.
 * @return A pointer to the loaded Cart struct, or NULL if loading fails.
 */
Cart *cart_load(const char *filename);

/**
 * @brief Frees the memory allocated for a cartridge.
 *
 * @param cart A pointer to the Cart struct to be freed.
 */
void cart_free(Cart *cart);

#endif // CART_H
