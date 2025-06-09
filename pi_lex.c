#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "pi_lex.h"

// Create an instance of scanner_t
scanner_t *scanner;

static void l_error(const char *message)
{
    fprintf(stderr, "Syntax Error: %s at line %d, column %d\n", message, scanner->line, scanner->column);
    exit(1);
}

// Function to initialize the scanner instance
void init_scanner(char *source)
{

    // Allocate memory for the scanner instance
    scanner = (scanner_t *)malloc(sizeof(scanner_t));

    scanner->source = source;

    scanner->size = 0;

    scanner->start = 0;
    scanner->current = 0;

    scanner->line = 1;
    scanner->column = 1;

    scanner->capacity = 128;

    scanner->is_negative = false;

    scanner->brackets = stack_create(sizeof(int));

    // Allocate memory for the tokens array with initial capacity
    scanner->tokens = (token_t *)malloc(scanner->capacity * sizeof(token_t));
}

// Function to scan the source code and return tokens
token_t *scan()
{
    scan_tokens();
    return scanner->tokens;
}

// Function to scan tokens from the source code
void scan_tokens()
{
    while (!is_AtEnd())
    {
        // We are at the beginning of the next lexeme.
        scanner->start = scanner->current;
        scan_token();
    }

    scanner->start = scanner->current;
    add_token(TK_EOF);
}

// Function to scan a token
void scan_token()
{
    scanner->ch = next();
    char _ch;

    switch (scanner->ch)
    {
    case '\r':
    case '\t':
    case ' ':
        break;
    case '\n':
        scanner->line++;
        scanner->column = 1;
        scanner->start = scanner->current;
        break;
    case '/':
        if (match('/'))
        {
            while (!is_AtEnd() && scanner->ch != '\n')
                scanner->ch = next();
            scanner->line++;
            scanner->column = 1;
            // scanner->start = scanner->current;
        }
        else if (match('*'))
        {
            while (!is_AtEnd())
            {
                if (scanner->ch == '*' && peek(0) == '/')
                    break;
                scanner->ch = next();
                if (scanner->ch == '\n')
                {
                    scanner->line++;
                    scanner->column = 1;
                }
            }
            if (!match('/'))
                l_error("Unclosed Comment");
            // scanner->start = scanner->current;
        }
        else if (match('='))
            add_token(TK_DIV_ASSIGN);
        else
            add_token(TK_DIV);
        break;
    case '[':
        add_token(TK_LBRACKET);
        break;
    case ']':
        add_token(TK_RBRACKET);
        break;
    case '{':
        add_token(TK_LBRACE);
        break;
    case '}':
        add_token(TK_RBRACE);
        break;
    case '(':
        add_token(TK_LPAREN);
        break;
    case ')':
        add_token(TK_RPAREN);
        break;
    case ';':
        add_token(TK_SEMICOLON);
        break;
    case ':':
        add_token(TK_COLON);
        break;
    case ',':
        add_token(TK_COMMA);
        break;
    case '?':
        add_token(TK_QUESTION);
        break;
    case '#':
        add_token(TK_HASH);
        break;
    case '=':
        if (match('='))
            add_token(TK_EQUAL);
        else
            add_token(TK_ASSIGN);
        break;
    case '*':
        if (match('*'))
            add_token(TK_POWER);
        else if (match('='))
            add_token(TK_MULT_ASSIGN);
        else if (match('.'))
            add_token(TK_DOT_PROD);
        else
            add_token(TK_MULT);
        break;

    case '@':
        if (match('='))
            add_token(TK_DOT_PROD_ASSIGN);
        else
            add_token(TK_DOT_PROD);
        break;
    case '+':
        if (match('='))
            add_token(TK_PLUS_ASSIGN);
        else if (match('+'))
            add_token(TK_INCR);
        else
            add_token(TK_PLUS);
        break;
    case '-':
        if (match('='))
            add_token(TK_MINUS_ASSIGN);
        else if (match('-'))
            add_token(TK_DECR);
        else if (match('>'))
            add_token(TK_RARROW);
        // else if (is_digit(peek(0)))
        // {
        //     scanner->is_negative = true;
        //     break;
        // }
        else
            add_token(TK_MINUS);
        break;
    case '%':
        if (match('='))
            add_token(TK_MOD_ASSIGN);
        else
            add_token(TK_MOD);
        break;
    case '|':
        if (match('='))
            add_token(TK_BITOR_ASSIGN);
        else if (match('|'))
            add_token(TK_OR);
        else
            add_token(TK_BITOR);
        break;
    case '&':
        if (match('='))
            add_token(TK_BITAND_ASSIGN);
        else if (match('&'))
            add_token(TK_AND);
        else
            add_token(TK_BITAND);
        break;
    case '^':
        if (match('='))
            add_token(TK_XOR_ASSIGN);
        else
            add_token(TK_XOR);
        break;
    case '~':
        add_token(TK_BITNEG);
        break;
    case '!':
        if (match('='))
            add_token(TK_NOT_EQUAL);
        else
            add_token(TK_NOT);
        break;
    case '<':
        if (match('='))
            add_token(TK_LESS_EQUAL);
        else if (match('<'))
            add_token(TK_LSHIFT);
        else if (match('-'))
            add_token(TK_LARROW);
        else
            add_token(TK_LESS);
        break;
    case '>':
        if (match('='))
            add_token(TK_GREATER_EQUAL);
        else if (match('>'))
        {
            if (match('>'))
            {
                if (match('='))
                    add_token(TK_URSHIFT_ASSIGN);
                else
                    add_token(TK_URSHIFT);
            }
            else
                add_token(TK_RSHIFT);
        }
        else
            add_token(TK_GREATER);
        break;
    case '"':
    case '\'':
        _ch = scanner->ch;
        while (!match(_ch))
        {
            next();
            if (match('\n'))
            {
                scanner->line++;
                scanner->column = 1;
            }
            else if (is_AtEnd())
                l_error("Unterminated String");
        }
        add_token(TK_STR);
        break;
    case '.':
        _ch = previous();
        scanner->ch = next();
        if (is_digit(scanner->ch) && _ch != ']' && !is_alpha(_ch))
        {
            decimal();
            add_token(TK_NUM);
        }
        else if (scanner->ch == '.')
        {
            scanner->ch = next();
            if (scanner->ch == '.')
                add_token(TK_ELLIPSIS);
            else
            {
                scanner->current--;
                add_token(TK_DBDOTS);
            }
        }
        else
        {
            scanner->current--;
            add_token(TK_DOT);
        }
        break;
    default:
        if (is_digit(scanner->ch))
        {

            if (scanner->ch == '0')
            {
                // Hex, Octal or Binary Numbers:
                // Hexadecimal Numbers
                if (match_s("xX"))
                {
                    do
                    {
                        if (!is_hexDigit(peek(0)))
                            l_error("invalid hexadecimal literal");
                        do
                        {
                            scanner->ch = next();
                        } while (is_hexDigit(peek(0)));
                    } while (match('_'));
                    add_token(TK_NUM);
                }
                else if (match_s("oO"))
                {
                    // parse octal number
                    do
                    {
                        if (!is_octDigit(peek(0)))
                            l_error("invalid octal literal");
                        scanner->ch = next();
                    } while (match('.') || is_digit(peek(0)));
                    add_token(TK_NUM);
                }
                else if (match_s("bB"))
                {
                    /* Binary */
                    do
                    {
                        if (!is_binDigit(peek(0)))
                            l_error("invalid binary literal");
                        scanner->ch = next();
                    } while (match('.') || is_digit(peek(0)));
                    add_token(TK_NUM);
                }
                else if (peek(0) == '.' && peek(1) != '.')
                {
                    scanner->ch = next();
                    if (is_digit(peek(0)))
                        decimal();
                    add_token(TK_NUM);
                }
                else if (is_digit(peek(0)))
                    l_error("leading zeros in decimal integer literals are not permitted");
                else
                    add_token(TK_NUM);
            }
            else
            {
                while (is_digit(peek(0)))
                    scanner->ch = next();
                if (peek(0) == '.' && peek(1) != '.')
                {
                    scanner->ch = next();
                    if (is_digit(peek(0)))
                        decimal();
                }
                add_token(TK_NUM);
            }
        }
        else if (is_alpha(scanner->ch))
        {
            while (is_validID(peek(0)))
                next();

            char *name = substring(scanner->source, scanner->start, scanner->current);
            tk_type type = find_kw(name);

            if (type == TK_INVALID)
                add_token(TK_ID);
            else
                add_token(type);

            free(name);
        }
        break;
    }
    // print_tokens();
}

// Function to get the next character from the source code
char next()
{
    scanner->current++;
    scanner->column++;
    return scanner->source[scanner->current - 1];
}

// Function to peek a character with an offset
char peek(int offset)
{
    if (scanner->current + offset >= strlen(scanner->source))
        return '\0';
    return scanner->source[scanner->current + offset];
}

bool match_s(const char *expected)
{
    if (is_AtEnd())
        return false;

    char ch = scanner->source[scanner->current];
    size_t len = strlen(expected);

    for (size_t i = 0; i < len; i++)
        if (ch == expected[i])
        {
            scanner->current++;
            scanner->column++;
            return true;
        }
    return false;
}

bool match(char expected)
{
    if (is_AtEnd())
        return false;
    if (scanner->source[scanner->current] != expected)
        return false;
    scanner->current++;
    scanner->column++;
    return true;
}

// Function to add a token to the scanner's token list
void add_token(tk_type type)
{

    char *start = scanner->source + scanner->start;
    int length = scanner->current - scanner->start;

    int index = scanner->size; // Current token index

    // Exclude quotes for string tokens
    if (type == TK_STR)
    {
        start++;     // Skip the opening quote
        length -= 2; // Exclude both the opening and closing quotes
    }
    // Ensure the tokens array has enough capacity
    if (scanner->size >= scanner->capacity)
    {
        scanner->capacity *= 2;
        scanner->tokens = (token_t *)realloc(scanner->tokens, scanner->capacity * sizeof(token_t));
    }

    // Create a token and add it directly to the tokens array
    token_t token = create_token(type, start, length, scanner->line, scanner->column);

    token.openAt = -1;
    token.closeAt = -1;

    // Handle bracket tracking
    switch (type)
    {
    case TK_LPAREN:
    case TK_LBRACE:
    case TK_LBRACKET:
        // Push opening bracket position (token index)
        push(scanner->brackets, &index);
        token.openAt = index;
        break;

    case TK_RPAREN:
    case TK_RBRACE:
    case TK_RBRACKET:
    {
        // Pop matching opening bracket
        if (!is_empty(scanner->brackets))
        {
            int *open_pos_ptr = pop(scanner->brackets);
            int open_pos = *open_pos_ptr;
            free(open_pos_ptr);

            // Link the brackets
            token.openAt = open_pos;
            scanner->tokens[open_pos].closeAt = index;
        }
        break;
    }
    }

    scanner->tokens[scanner->size++] = token;
}

bool is_AtEnd()
{
    return scanner->current >= strlen(scanner->source);
}

bool is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool is_hexDigit(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

bool is_octDigit(char ch)
{
    return ch >= '0' && ch <= '7';
}

bool is_binDigit(char ch)
{
    return ch == '0' || ch == '1';
}

double parse_hex(const char *num)
{
    double val = 0.0;
    for (int i = 2; num[i] != '\0'; i++)
    {
        char c = toupper(num[i]);
        if (c != '_' && c != '.')
        {
            int d = (c >= '0' && c <= '9') ? c - '0' : c - 'A' + 10;
            val = 16 * val + d;
        }
    }
    return val;
}

double parse_oct(const char *num)
{
    double val = 0.0;
    const char *digits = "01234567";
    int start_index = (num[1] == 'O' || num[1] == 'o') ? 2 : 1;
    for (int i = start_index; num[i] != '\0'; i++)
    {
        char c = num[i];
        if (c != '_')
        {
            int d = strchr(digits, c) - digits;
            val = 8 * val + d;
        }
    }
    return val;
}

double parse_bin(const char *num)
{
    double val = 0.0;
    for (int i = 2; num[i] != '\0'; i++)
        val = 2 * val + (num[i] == '1' ? 1.0 : 0.0);

    return val;
}

void decimal()
{
    while (is_digit(peek(0)))
        next();
    if (match_s("eE"))
    {
        if (match_s("+-"))
        {
            next();
            if (!is_digit(peek(0)))
                l_error("invalid decimal literal");
            while (is_digit(peek(0)))
                next();
        }
        else if (!is_digit(peek(0)))
            l_error("invalid decimal literal");
        while (is_digit(peek(0)))
            next();
    }
}

bool is_alpha(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

// check if the passed character is a valid character for an identifier
bool is_validID(char ch)
{
    return is_alpha(ch) || is_digit(ch);
}

char *get_space(int count)
{
    char *space = (char *)malloc(count + 1);
    for (int i = 0; i < count; i++)
        space[i] = ' ';
    space[count] = '\0';
    return space;
}

bool is_space(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r';
}

char previous()
{
    int i = scanner->current - 2;
    for (; i >= 0; i--)
        if (!is_space(scanner->source[i]))
            return scanner->source[i];

    return '\0';
}

// Function to free the scanner and its tokens
void free_scanner()
{
    // Free the memory allocated for the array of token pointers
    if (scanner->tokens != NULL)
        free(scanner->tokens); // Free the entire array

    // Free the memory allocated for the scanner itself
    free(scanner);
}

char *substring(const char *source, int start, int end)
{
    int length = end - start;
    char *str = (char *)malloc(length + 1);
    if (!str)
    {
        perror("Failed to allocate memory for substring");
        exit(EXIT_FAILURE);
    }

    memcpy(str, source + start, length);
    str[length] = '\0'; // Null-terminate the substring

    return str;
}