#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pi_string.h"
#include "../pi_object.h"

/**
 * @brief Return a single character string based on the given numeric argument.
 *
 * @param vm The virtual machine.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return A single character string.
 */
Value pi_char(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !is_numeric(argv[0]))
        error("[char] expects a single numeric argument.");

    char *result = (char *)malloc(2);
    result[0] = (char)as_number(argv[0]);
    result[1] = '\0';

    return NEW_OBJ(new_pistring(result));
}

/**
 * Returns the Unicode code point (ASCII value) of the first character in the string.
 * If the input is not a string or is empty, an error is raised.
 */
Value pi_ord(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[ord] expects a non-empty string as argument.");

    PiString *str = AS_STRING(argv[0]);

    if (str->length == 0)
        error("[ord] cannot operate on an empty string.");

    unsigned char ch = str->chars[0];
    return NEW_NUM((double)ch);
}

/**
 * Removes leading and trailing whitespace from the input string.
 * Returns a new trimmed string.
 */
Value pi_trim(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[trim] expects a string argument.");

    PiString *str = AS_STRING(argv[0]);
    char *s = str->chars;
    int start = 0;
    int end = str->length - 1;

    // Skip leading whitespace
    while (start <= end && isspace((unsigned char)s[start]))
        start++;

    // Skip trailing whitespace
    while (end >= start && isspace((unsigned char)s[end]))
        end--;

    int new_length = end - start + 1;
    if (new_length <= 0)
        return NEW_OBJ(copy_pistring("", 0)); // All spaces

    return NEW_OBJ(copy_pistring(&s[start], new_length));
}

/**
 * Converts all characters in the string to uppercase.
 * Returns a new uppercase string.
 */
Value pi_upper(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[upper] expects a string argument.");

    PiString *str = AS_STRING(argv[0]);
    int len = str->length;

    char *upper_str = malloc(len + 1);
    if (!upper_str)
        error("[upper] Memory allocation failed.");

    for (int i = 0; i < len; i++)
        upper_str[i] = toupper((unsigned char)str->chars[i]);

    upper_str[len] = '\0';
    Value result = NEW_OBJ(copy_pistring(upper_str, len));
    free(upper_str);
    return result;
}

/**
 * Converts all characters in the string to lowercase.
 * Returns a new lowercase string.
 */
Value pi_lower(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[lower] expects a string argument.");

    PiString *str = AS_STRING(argv[0]);
    int len = str->length;

    char *lower_str = malloc(len + 1);
    if (!lower_str)
        error("[lower] Memory allocation failed.");

    for (int i = 0; i < len; i++)
        lower_str[i] = tolower((unsigned char)str->chars[i]);

    lower_str[len] = '\0';
    Value result = NEW_OBJ(copy_pistring(lower_str, len));
    free(lower_str);
    return result;
}

/**
 * Replaces all occurrences of `old` with `new` in the given string.
 * Returns a new string with replacements applied.
 */
Value pi_replace(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3 || !IS_STRING(argv[0]) || !IS_STRING(argv[1]) || !IS_STRING(argv[2]))
        error("[replace] expects three string arguments: (str, old, new).");

    PiString *source = AS_STRING(argv[0]);
    PiString *old_sub = AS_STRING(argv[1]);
    PiString *new_sub = AS_STRING(argv[2]);

    const char *src = source->chars;
    const char *old_str = old_sub->chars;
    const char *new_str = new_sub->chars;

    if (old_sub->length == 0)
        error("[replace] 'old' string must not be empty.");

    // Estimate maximum length needed
    size_t new_len_estimate = source->length * 2 + 1;
    char *result = malloc(new_len_estimate);
    if (!result)
        error("[replace] Memory allocation failed.");

    size_t src_index = 0, res_index = 0;
    while (src[src_index])
    {
        if (strncmp(&src[src_index], old_str, old_sub->length) == 0)
        {
            memcpy(&result[res_index], new_str, new_sub->length);
            res_index += new_sub->length;
            src_index += old_sub->length;
        }
        else
            result[res_index++] = src[src_index++];
    }
    result[res_index] = '\0';

    Value final = NEW_OBJ(copy_pistring(result, res_index));
    free(result);
    return final;
}

/**
 * Returns true if all alphabetic characters in the string are uppercase.
 *
 * Example:
 *   is_upper("HELLO") => true
 *   is_upper("Hello") => false
 *   is_upper("123!")  => true (no lowercase letters)
 */
Value pi_isUpper(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_upper] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;

    for (int i = 0; i < str->length; i++)
    {
        if (isalpha(s[i]) && !isupper(s[i]))
            return NEW_BOOL(false);
    }

    return NEW_BOOL(true);
}

/**
 * Returns true if all alphabetic characters in the string are lowercase.
 *
 * Example:
 *   is_lower("hello") => true
 *   is_lower("Hello") => false
 *   is_lower("123!")  => true (no uppercase letters)
 */
Value pi_isLower(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_lower] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;

    for (int i = 0; i < str->length; i++)
    {
        if (isalpha(s[i]) && !islower(s[i]))
            return NEW_BOOL(false);
    }

    return NEW_BOOL(true);
}

/**
 * Returns true if all characters in the string are digits (0-9).
 *
 * Example:
 *   is_digit("12345") => true
 *   is_digit("123a5") => false
 *   is_digit("")      => false (empty string)
 */
Value pi_isDigit(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_digit] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;

    if (str->length == 0)
        return NEW_BOOL(false);

    for (int i = 0; i < str->length; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return NEW_BOOL(false);
    }

    return NEW_BOOL(true);
}

/**
 * Returns true if the given string represents a valid numeric value (integer or floating point).
 *
 * Examples:
 *   is_numeric("123")    => true
 *   is_numeric("-45.67") => true
 *   is_numeric("abc123") => false
 *   is_numeric("")       => false
 */
Value pi_isNumeric(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_numeric] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;
    int len = str->length;

    if (len == 0)
        return NEW_BOOL(false);

    int i = 0;
    int dot_count = 0;

    // Optional leading sign
    if (s[i] == '+' || s[i] == '-')
        i++;

    if (i == len) // Only sign, no digits
        return NEW_BOOL(false);

    for (; i < len; i++)
    {
        if (s[i] == '.')
        {
            dot_count++;
            if (dot_count > 1)
                return NEW_BOOL(false); // More than one dot
        }
        else if (!isdigit((unsigned char)s[i]))
            return NEW_BOOL(false);
    }

    return NEW_BOOL(true);
}

/**
 * Returns true if the given string contains only alphabetic characters (A-Z, a-z).
 *
 * Examples:
 *   is_alpha("Hello")  => true
 *   is_alpha("abc123") => false
 *   is_alpha("")       => false
 */
Value pi_isAlpha(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_alpha] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;
    int len = str->length;

    if (len == 0)
        return NEW_BOOL(false);

    for (int i = 0; i < len; i++)
    {
        if (!isalpha((unsigned char)s[i]))
        {
            return NEW_BOOL(false);
        }
    }

    return NEW_BOOL(true);
}

/**
 * Returns true if the given string contains only alphanumeric characters (A-Z, a-z, 0-9).
 *
 * Examples:
 *   is_alnum("Hello123") => true
 *   is_alnum("abc_123")  => false
 *   is_alnum("")         => false
 */
Value pi_isAlnum(vm_t *vm, int argc, Value *argv)
{
    if (argc == 0 || !IS_STRING(argv[0]))
        error("[is_alnum] expects a string as argument.");

    PiString *str = AS_STRING(argv[0]);
    const char *s = str->chars;
    int len = str->length;

    if (len == 0)
        return NEW_BOOL(false);

    for (int i = 0; i < len; i++)
    {
        if (!isalnum((unsigned char)s[i]))
        {
            return NEW_BOOL(false);
        }
    }

    return NEW_BOOL(true);
}

Value pi_split(vm_t *vm, int argc, Value *argv)
{
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1]))
        error("[split] expects two string arguments.");

    const char *str = AS_CSTRING(argv[0]);
    const char *delim = AS_CSTRING(argv[1]);

    size_t len = strlen(str);
    list_t *result = list_create(sizeof(Value));

    if (len == 0)
        return NEW_OBJ(new_list(result));

    char *token = strtok((char *)str, delim);

    int i = 0;
    while (token != NULL)
    {
        list_add(result, &NEW_OBJ(new_pistring(token)));
        token = strtok(NULL, delim);
        i++;
    }

    return NEW_OBJ(new_list(result));
}