
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

// Xorshift32 PRNG function
static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

// Function to generate a random double in the range [0, 1]
double rand_num(uint32_t *state)
{
    return (double)xorshift32(state) / UINT32_MAX;
}

char *itos(int num)
{
    // Determine the length of the number
    int length = 0;
    int temp = num;

    // Handle the case of 0 explicitly
    if (temp == 0)
        length = 1;
    else
        // Calculate the length of the number
        while (temp != 0)
        {
            length++;
            temp /= 10;
        }

    // Allocate memory for the string (including space for the null terminator)
    char *str = (char *)malloc((length + 1) * sizeof(char));

    // Handle negative numbers
    if (num < 0)
    {
        str[0] = '-';
        num = -num;
    }

    // Convert each digit to a character
    int i = length - 1;
    do
    {
        str[i] = (num % 10) + '0';
        num /= 10;
        i--;
    } while (num != 0);

    // Null-terminate the string
    str[length] = '\0';

    return str;
}

void error(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    fflush(stdout); // Make sure stdout is flushed before printing to stderr
    fprintf(stderr, "[Error] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(EXIT_FAILURE);
}