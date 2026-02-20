#include "pi_min.h"

/**
 * Checks if a given character is a valid token character.
 * A token character is either alphanumeric, or one of the following
 * characters: _, $, -, <.
 * @param c The character to check.
 * @return true if it is a token character, false otherwise.
 */
static bool is_identifier(char c)
{
    return isalnum(c) || c == '_' || c == '$';
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

/**
 * Determines if a semicolon is needed after a character.
 *
 * A semicolon is typically required after specific characters
 * or alphanumeric characters in the code.
 *
 * @param c The character to check.
 * @return true if a semicolon is needed after the character, false otherwise.
 */
static bool needs_semicolon(char c)
{
    // Check if the character is one of the specific symbols or is alphanumeric
    return c == ')' || c == ']' || c == '"' || c == '\'' || c == '}' || isalnum(c);
}

/**
 * Checks if the given position is a control statement keyword.
 * @param code The input code being minimized.
 * @param pos The position in the input code to check.
 * @return true if the position is a control statement keyword, false otherwise.
 */
static bool is_controlStatement(const char *code, int pos)
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

/**
 * Checks if the given character is a continuation character.
 * Continuation characters are operators that indicate that the
 * current expression continues to the next line.
 * @param c The character to check.
 * @return true if the character is a continuation character, false otherwise.
 */
static bool is_continuation(char c)
{
    // Operators that are continuation characters
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '&' || c == '|' || c == '^' || c == '.' || c == ',' ||
           c == ':' || c == '<' || c == '>' || c == '=' || c == '!';
}

/**
 * --- Phase 1: Remove comments ---
 *
 * This function processes both single-line (//) and multi-line (/* * /) comments,
 * returning a new string with all comments removed.
 *
 * @param input The input source code as a string.
 * @return A new string with comments removed. The caller is responsible for freeing the memory.
 */
static char *remove_comments(const char *input)
{
    int len = strlen(input);
    char *output = malloc(len + 1);
    if (!output)
        return NULL;

    int i = 0, j = 0;
    bool in_string = false;
    char _char = 0;

    while (input[i])
    {
        // Start or end of string
        if (!in_string && (input[i] == '"' || input[i] == '\''))
        {
            in_string = true;
            _char = input[i];
            output[j++] = input[i++];
        }
        else if (in_string)
        {
            // Handle escape characters
            if (input[i] == '\\' && input[i + 1])
            {
                output[j++] = input[i++];
                output[j++] = input[i++];
            }
            else
            {
                output[j++] = input[i];
                if (input[i] == _char)
                {
                    in_string = false;
                    _char = 0;
                }
                i++;
            }
        }
        // Handle single-line comment
        else if (input[i] == '/' && input[i + 1] == '/')
        {
            i += 2;
            while (input[i] && input[i] != '\n')
                i++;
        }
        // Handle multi-line comment
        else if (input[i] == '/' && input[i + 1] == '*')
        {
            i += 2;
            while (input[i] && !(input[i] == '*' && input[i + 1] == '/'))
                i++;
            if (input[i])
                i += 2;
        }
        else
            // Copy regular character
            output[j++] = input[i++];
    }

    output[j] = '\0';
    return realloc(output, j + 1);
}

/**
 * Minimizes the input source code by removing unnecessary whitespaces,
 * adding semicolons where necessary, and handling control structures.
 *
 * @param input The input source code as a string.
 * @param output The output buffer to store the minimized code.
 */

char *minimize(const char *input)
{
    char *clean = remove_comments(input);
    if (!clean)
        return NULL;

    int capacity = strlen(clean) * 2 + 1;
    char *buffer = malloc(capacity);
    if (!buffer)
    {
        free(clean);
        return NULL;
    }

    int i = 0, j = 0, paren_depth = 0;
    char prev = 0;
    bool skip_semicolon = false, add_paren = false, is_increment = false;
    int bracket_index = -1;

    while (clean[i])
    {
        if (clean[i] == '=')
        {
            buffer[j++] = clean[i++];
            int depth = 0;
            bool is_bracket = false;
            while (is_space(clean[i]))
                i++;
            int k = i;
            do
            {
                if (clean[k] == '{')
                {
                    depth++;
                    is_bracket = true;
                }
                else if (clean[k] == '}')
                    depth--;
                k++;
            } while (depth > 0);

            if (is_bracket)
                bracket_index = k - 1;
            continue;
        }

        if (clean[i] == '(')
            paren_depth++;
        else if (clean[i] == ')')
        {
            paren_depth--;
            if (paren_depth == 0 && skip_semicolon)
            {
                int k = i + 1;
                while (is_space(clean[k]) && clean[k] != '\n')
                    k++;
                if (clean[k] != '\n')
                    skip_semicolon = false;
            }
        }

        if (clean[i] == '{' && add_paren)
        {
            buffer[j++] = ')';
            add_paren = false;
            paren_depth--;
        }

        if (clean[i] == '}')
        {
            if (bracket_index == i)
            {
                buffer[j++] = clean[i++];
                while (is_space(clean[i]) && clean[i] != '\n')
                    i++;
                if (clean[i] == '\n')
                {
                    buffer[j++] = ';';
                    prev = ';';
                }
                bracket_index = -1;
                continue;
            }
        }

        if (is_controlStatement(clean, i))
        {
            skip_semicolon = true;
            int start = i;
            while (isalpha(clean[i]))
                buffer[j++] = clean[i++];

            int len = i - start;
            if (len == 4 && strncmp(&clean[start], "else", 4) == 0)
            {
                buffer[j++] = ' ';
                continue;
            }

            while (is_space(clean[i]))
                i++;

            if (clean[i] != '(')
            {
                buffer[j++] = '(';
                add_paren = true;
            }
            paren_depth++;
        }

        if (clean[i] == '"' || clean[i] == '\'')
        {
            char quote = clean[i];
            buffer[j++] = clean[i++];
            while (clean[i] && (clean[i] != quote || clean[i - 1] == '\\'))
                buffer[j++] = clean[i++];
            if (clean[i])
                buffer[j++] = clean[i++];
            prev = quote;
            continue;
        }

        if (clean[i] == '\n')
        {
            if (add_paren && !is_continuation(prev))
            {
                buffer[j++] = ')';
                add_paren = false;
                paren_depth--;
            }

            while (is_space(clean[i]))
                i++;

            char next = clean[i];

            bool continuation = is_continuation(next);
            bool is_increment = (prev == '+' && next == '+') || (prev == '-' && next == '-');

            if (!continuation &&
                !is_increment &&
                !skip_semicolon &&
                paren_depth == 0 &&
                needs_semicolon(prev) &&
                prev != ';' && prev != '}' &&
                next != '}' && next != ']')
            {
                buffer[j++] = ';';
                prev = ';';
            }

            skip_semicolon = false;
            continue;
        }

        if (is_space(clean[i]))
        {
            if ((is_identifier(prev) && is_identifier(clean[i + 1])) ||
                (prev == '<' && clean[i + 1] == '-'))
                buffer[j++] = ' ';
            i++;
            continue;
        }

        buffer[j++] = clean[i];
        prev = clean[i];
        i++;
    }

    free(clean); // Done with phase 1 buffer

    // Now resize to actual output length
    char *output = realloc(buffer, j + 1); // +1 for null terminator
    if (!output)
    {
        // realloc can fail; fall back to the original buffer
        buffer[j] = '\0';
        return buffer;
    }

    output[j] = '\0';
    return output;
}
