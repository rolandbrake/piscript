
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Checks if a given character is a valid token character.
 * A token character is either alphanumeric, or one of the following
 * characters: _, $, -, <.
 * @param c The character to check.
 * @return true if it is a token character, false otherwise.
 */
static bool is_tokenChar(char c)
{
    return isalnum(c) || c == '_' || c == '$' || c == '-' || c == '<';
}

/**
 * Checks if a given character is a whitespace character.
 * @param c The character to check.
 * @return true if it is a space, false otherwise.
 */
static bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool needs_semicolon(char c)
{
    return c == ')' || c == ']' || c == '"' || c == '\'' || c == '}' || isalnum(c);
}

/**
 * Checks if the given position is a control statement keyword.
 * @param code The input code being minimized.
 * @param pos The position in the input code to check.
 * @return true if the position is a control statement keyword, false otherwise.
 */
static bool is_control_statement(const char *code, int pos)
{
    // Keywords that are control statements.
    const char *keywords[] = {"if", "for", "while", "else", "elif"};
    const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

    for (int k = 0; k < keyword_count; k++)
    {
        int len = strlen(keywords[k]);
        if (strncmp(&code[pos], keywords[k], len) == 0)
        {
            // Ensure it's not part of a larger word (e.g., "elseif")
            if (!isalnum(code[pos + len]))
                return true;
        }
    }
    return false;
}

static void minimize(const char *input, char *output)
{
    int i = 0, j = 0;
    char prev = 0;
    bool suppress_semicolon = false;
    bool add_parentheses = false;
    int paren_depth = 0;

    while (input[i])
    {

        if (input[i] == '(')
            paren_depth++;
        else if (input[i] == ')')
            paren_depth--;

        if (input[i] == '{' && add_parentheses)
        {
            output[j++] = ')';
            add_parentheses = false;
            paren_depth--;
        }

        if (is_control_statement(input, i))
        {
            suppress_semicolon = true;

            int start = i;

            while (isalpha(input[i]))
                output[j++] = input[i++];

            // Check for "else" or "elif" by comparing the keyword from 'start' to current 'i'
            int len = i - start;

            if ((len == 4 && strncmp(&input[start], "else", 4) == 0) ||
                (len == 4 && strncmp(&input[start], "elif", 4) == 0))
            {
                output[j++] = ' ';
                // Do NOT insert parentheses for else/elif
                continue;
            }

            while (is_space(input[i]))
                i++;

            if (input[i] != '(')
            {
                output[j++] = '(';
                add_parentheses = true;
            }
            paren_depth++;
        }

        // Skip comments (same as before)
        if (input[i] == '/' && input[i + 1] == '/')
        {
            i += 2;
            while (input[i] && input[i] != '\n')
                i++;
            continue;
        }
        if (input[i] == '/' && input[i + 1] == '*')
        {
            i += 2;
            while (input[i] && !(input[i] == '*' && input[i + 1] == '/'))
                i++;
            if (input[i])
                i += 2;
            continue;
        }

        // Handle strings (same as before)
        if (input[i] == '"' || input[i] == '\'')
        {
            char quote = input[i];
            output[j++] = input[i++];
            while (input[i] && (input[i] != quote || input[i - 1] == '\\'))
                output[j++] = input[i++];

            if (input[i])
                output[j++] = input[i++];
            prev = quote;
            continue;
        }

        // Handle newlines (only insert semicolon if needed and not suppressed)
        if (input[i] == '\n')
        {
            if (add_parentheses)
            {
                output[j++] = ')';
                add_parentheses = false;
                paren_depth--;
            }            

            if (needs_semicolon(prev) && !suppress_semicolon && prev != ';' && prev != '}' && paren_depth == 0)
            {
                output[j++] = ';';
                prev = ';';
            }
            suppress_semicolon = false; // Reset after newline

            i++;
            continue;
        }

        // Remove extra whitespace (same as before)
        if (is_space(input[i]))
        {
            if (is_tokenChar(prev) && is_tokenChar(input[i + 1]))
                output[j++] = ' ';
            i++;
            continue;
        }

        // Normal character
        output[j++] = input[i];
        prev = input[i];
        i++;
    }

    output[j] = '\0';
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s input.pis output.pi\n", argv[0]);
        return 1;
    }

    // Read entire input file
    FILE *fin = fopen(argv[1], "rb");
    if (!fin)
    {
        perror("Cannot open input");
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    long size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    char *input = malloc(size + 1);
    fread(input, 1, size, fin);
    input[size] = '\0';
    fclose(fin);

    // Allocate for output
    char *output = malloc(size * 2); // more than enough

    // Run minimizer
    minimize(input, output);

    // Write to output file
    FILE *fout = fopen(argv[2], "wb");
    if (!fout)
    {
        perror("Cannot open output");
        return 1;
    }
    fwrite(output, 1, strlen(output), fout);
    fclose(fout);

    printf("Minimized %s to %s successfully.\n", argv[1], argv[2]);

    free(input);
    free(output);
    return 0;
}
