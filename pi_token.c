
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "pi_token.h"

token_t create_token(tk_type type, char *start, int length, int line, int column)
{
    token_t token;

    // Allocate memory for the token's lexeme and copy it
    token.start = (char *)malloc(length + 1);
    if (!token.start)
    {
        perror("Failed to allocate memory for token lexeme");
        exit(EXIT_FAILURE);
    }

    memcpy(token.start, start, length);
    token.start[length] = '\0'; // Ensure null termination

    // Set the other token properties
    token.type = type;
    token.length = length;
    token.line = line;
    token.column = column;
    token.is_negative = false;
    token.skip = false;

    return token;
}

tk_type token_type(token_t token)
{
    return token.type;
}

// Function to get the value of the token as a string
char *token_value(token_t token)
{
    int length = token.length;
    char *start = token.start;

    // Check if the token is negative and adjust memory allocation
    int _length = token.is_negative ? 1 : 0;      // Add 1 for '-' if negative
    char *new_str = malloc(length + _length + 1); // +1 for null terminator

    if (new_str)
    {
        // If negative, add the minus sign to the beginning
        if (token.is_negative)
        {
            new_str[0] = '-';
            strncpy(new_str + 1, start, length); // Copy the original token after '-'
        }
        else
            strncpy(new_str, start, length); // Copy the original token
        new_str[length + _length] = '\0';    // Set null terminator
    }

    return new_str;
}

int token_line(token_t token)
{
    return token.line;
}

int token_column(token_t token)
{
    return token.column;
}

// Function to convert token to string (simple implementation)
const char *token_toString(token_t token)
{
    // Allocate enough space to hold the type, token value, and additional characters
    // Adjust the buffer size as needed to fit your largest expected token value
    int buffer_size = 256;
    char *result = (char *)malloc(buffer_size);
    if (token.length == 0)
        snprintf(result, buffer_size, "<%s>", token_names[token.type]);
    else
        snprintf(result, buffer_size, "<%s, %.*s>", token_names[token.type], token.length, token.start);

    return result;
}

tk_type find_kw(const char *name)
{
    for (int i = 0; i < KW_NUM; ++i)
        if (strcmp(keywords[i].name, name) == 0)
            return keywords[i].type;
    return TK_INVALID;
}

double tk_double(const token_t token)
{
    // Allocate a buffer large enough to hold the token string plus a null terminator.
    char *buffer = malloc(token.length + 1);

    // Copy the token characters into the buffer and null-terminate it.
    memcpy(buffer, token.start, token.length);
    buffer[token.length] = '\0';

    // Convert the string to a double.
    double result = strtod(buffer, NULL);

    // Clean up the temporary buffer.
    free(buffer);

    return token.is_negative ? -result : result;
}

char *tk_string(const token_t token)
{
    // char *buffer = (char *)malloc(token.length + 1);
    // snprintf(buffer, token.length + 1, "%.*s", token.length, token.start);
    // return buffer; // The caller is responsible for freeing this memory
    return token_value(token);
}

bool tk_bool(const token_t token)
{
    // if (strncmp(token.start, "true", 4) == 0)
    //     return true;
    // else if (strncmp(token.start, "false", 5) == 0)
    //     return false;
    // else
    // {
    //     fprintf(stderr, "Invalid boolean token: %.*s\n", token.length, token.start);
    //     return false; // Default return value for invalid input
    // }

    char *str = token_value(token);
    if (strcmp(str, "true") == 0)
        return true;
    else
        return false;
}
