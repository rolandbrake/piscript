#ifndef PI_LEX_H
#define PI_LEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pi_token.h"
#include "pi_stack.h"
#include "pi_table.h"

/**
 * Struct representing the lexical scanner.
 * The scanner is responsible for tokenizing the input source code.
 */
typedef struct
{
    char *source;    // Pointer to the source code string
    token_t *tokens; // Array of tokens generated from the source code

    int capacity; // Maximum capacity of the tokens array
    int size;     // Current number of tokens stored

    int start;   // Start position of the current token
    int current; // Current position in the source string

    bool is_negative; // Flag to track if a number is negative

    int line;   // Current line number in the source code
    int column; // Current column number in the source code

    char ch; // The current character being processed
    stack_t *brackets;
} scanner_t;

/**
 * Initializes the scanner with the given source code.
 * @param source The source code string to be tokenized.
 */
void init_scanner(char *source);

/**
 * Retrieves the next token from the source code.
 * @return Pointer to the token structure.
 */
token_t *scan();

/**
 * Processes all tokens from the source code.
 */
void scan_tokens();

/**
 * Scans and processes a single token.
 */
void scan_token();

/**
 * Advances the scanner and returns the next character.
 * @return The next character in the source code.
 */
char next();

/**
 * Peeks ahead in the source code without advancing.
 * @param offset The number of positions to look ahead.
 * @return The character at the given offset.
 */
char peek(int offset);

/**
 * Matches a multi-character string against the expected value.
 * @param expected The string to match.
 * @return true if matched, false otherwise.
 */
bool match_s(const char *expected);

/**
 * Matches the next character against the expected value.
 * @param expected The expected character.
 * @return true if matched, false otherwise.
 */
bool match(char expected);

/**
 * Adds a token to the list of tokens.
 * @param type The type of token to be added.
 */
void add_token(tk_type type);

/**
 * Checks if the scanner has reached the end of the source code.
 * @return true if at the end, false otherwise.
 */
bool is_AtEnd();

/**
 * Checks if the given character is a decimal digit (0-9).
 * @param ch The character to check.
 * @return true if it is a digit, false otherwise.
 */
bool is_digit(char ch);

/**
 * Checks if the given character is a hexadecimal digit (0-9, A-F, a-f).
 * @param ch The character to check.
 * @return true if it is a hex digit, false otherwise.
 */
bool is_hexDigit(char ch);

/**
 * Checks if the given character is an octal digit (0-7).
 * @param ch The character to check.
 * @return true if it is an octal digit, false otherwise.
 */
bool is_octDigit(char ch);

/**
 * Checks if the given character is a binary digit (0 or 1).
 * @param ch The character to check.
 * @return true if it is a binary digit, false otherwise.
 */
bool is_binDigit(char ch);

/**
 * Parses a hexadecimal number string and converts it to a double.
 * @param num The hexadecimal number string.
 * @return The decimal equivalent as a double.
 */
double parse_hex(const char *num);

/**
 * Parses an octal number string and converts it to a double.
 * @param num The octal number string.
 * @return The decimal equivalent as a double.
 */
double parse_oct(const char *num);

/**
 * Parses a binary number string and converts it to a double.
 * @param num The binary number string.
 * @return The decimal equivalent as a double.
 */
double parse_bin(const char *num);

/**
 * Processes a decimal number in the source code.
 */
void decimal();

/**
 * Checks if the given character is an alphabetical letter (A-Z, a-z, _).
 * @param ch The character to check.
 * @return true if it is an alphabetic character, false otherwise.
 */
bool is_alpha(char ch);

/**
 * Checks if the given character is a valid identifier character (A-Z, a-z, _, 0-9).
 * @param ch The character to check.
 * @return true if it is a valid identifier character, false otherwise.
 */
bool is_validID(char ch);

/**
 * Allocates a space-padded string of the given length.
 * @param count The number of spaces required.
 * @return Pointer to the allocated space string.
 */
char *get_space(int count);

/**
 * Checks if the given character is a whitespace character.
 * @param ch The character to check.
 * @return true if it is a space, false otherwise.
 */
bool is_space(char ch);

/**
 * Retrieves the previous character in the source code.
 * @return The previous character.
 */
char previous();

/**
 * Frees the memory allocated for the scanner.
 */
void free_scanner();

/**
 * Extracts a substring from the source code.
 * @param source The original source string.
 * @param start The starting index of the substring.
 * @param end The ending index of the substring.
 * @return A newly allocated substring.
 */
char *substring(const char *source, int start, int end);

#endif // PI_LEX_H
