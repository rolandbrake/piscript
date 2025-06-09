#ifndef PI_PARSER_H
#define PI_PARSER_H

#include <stdbool.h>
#include "pi_token.h"
#include "pi_compiler.h"
/**
 * Parser state structure.
 */
typedef struct
{
    token_t *tokens; // Array of tokens being parsed.
    token_t last;    // Last token parsed.
    int current;     // Current token index being parsed.
    bool access;     // Flag indicating whether access to variables is allowed.
    bool is_store;   // Flag indicating whether a store operation is in progress.
    bool emit_load;  // Flag indicating whether to emit a LOAD instruction.
    bool is_assign;  // Flag indicating whether an assignment is in progress.
    bool has_walrus; // Flag indicating whether a walrus operator is in progress.
    tk_type op;      // Type of operator being parsed (e.g. TK_PLUS, TK_MINUS, etc.).

    // Compiler associated with the parser
    compiler_t *comp;

    bool is_return;

} parser_t;

/**
 * Assignment operation structure.
 */
typedef struct
{
    int left;   // Index of the left-hand side of the assignment.
    int right;  // Index of the right-hand side of the assignment.
    tk_type op; // Type of the assignment operator (e.g. TK_ASSIGN, TK_PLUS_EQUAL, etc.).

} assign_t;

parser_t *init_parser(compiler_t *comp, token_t *tokens);
void parse(parser_t *parser);
void free_parser(parser_t *parser);

#endif