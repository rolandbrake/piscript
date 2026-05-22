#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "pi_parser.h"
#include "pi_compiler.h"
#include "pi_opcode.h"
#include "pi_object.h"
#include "pi_string.h"

// Operator definitions for parsing expressions
char *comp_ops[] = {"==", "!=", ">", "<", ">=", "<=", "in"};
char *bin_ops[] = {"+", "-", "*", "/", "%", "&&", "||", "**", "&", "|", "^", "<<", ">>", ">>>", ".", "is"};
char *unary_ops[] = {"+", "-", "!", "~", "#", "++", "--", "typeof"};

// Function prototypes for static functions
static void program(parser_t *parser);
static void declaration(parser_t *parser);
static void var_decl(parser_t *parser);
static void func_decl(parser_t *parser);
static void class_decl(parser_t *parser);
static void statement(parser_t *parser);
static void expr_state(parser_t *parser);
static void destructure_assignStatement(parser_t *parser);
static void block(parser_t *parser);
static void if_stmt(parser_t *parser);
static void while_stmt(parser_t *parser);
static void for_stmt(parser_t *parser);
static void break_stmt(parser_t *parser);
static void continue_stmt(parser_t *parser);
static void return_stmt(parser_t *parser);
static void print(parser_t *parser);
static void variable(parser_t *parser);
static void expr(parser_t *parser);
static void assignment(parser_t *parser, bool emit_load);
static void cond_expr(parser_t *parser);
static void or_expr(parser_t *parser);
static void and_expr(parser_t *parser);
static void in_expr(parser_t *parser);
static void range_expr(parser_t *parser);
static void bitOr_expr(parser_t *parser);
static void xor_expr(parser_t *parser);
static void bitAnd_expr(parser_t *parser);
static void shift_expr(parser_t *parser);
static void equality_expr(parser_t *parser);
static void compare_expr(parser_t *parser);
static void add_expr(parser_t *parser);
static void dot_expr(parser_t *parser);
static void mult_expr(parser_t *parser);
static void exp_expr(parser_t *parser);
static void member_expr(parser_t *parser);
static void unary_expr(parser_t *parser);
static void primary(parser_t *parser);

static void emit_spreadListLiteral(parser_t *parser);
static void emit_spreadMapLiteral(parser_t *parser);
static void emit_classMap(parser_t *parser, const char *class_name);
static void emit_boundMethodCall(parser_t *parser, const char *receiver, const char *method, int argc);
static void emit_listComprehension(parser_t *parser);

static bool call_hasSpreadArgs(parser_t *parser);

static bool list_hasSpreadItems(parser_t *parser);
static bool list_isComprehension(parser_t *parser);

static bool map_hasSpreadItems(parser_t *parser);
static bool is_mapEntry(parser_t *parser);
static void emit_setLiteral(parser_t *parser);
static bool has_accessContinuation(parser_t *parser, token_t token);

static token_t peek(parser_t *parser);
static bool check(parser_t *parser, tk_type type);
static bool match(parser_t *parser, tk_type type);
static token_t consume(parser_t *parser, tk_type type, const char *message);
static void advance(parser_t *parser);
void set_pos(parser_t *parser, token_t token);
static bool is_functionLiteral(parser_t *parser, int index);
static bool is_objectLiteral(parser_t *parser, int index);
static char *get_pendingFunctionName(parser_t *parser);
static void emit_mapFinalize(parser_t *parser);

typedef struct
{
    int start;
    int end;
} segment_t;

typedef struct
{
    token_t name;
    segment_t iterable;
} comp_iter_t;

typedef struct
{
    segment_t result;
    segment_t iterators;
    segment_t conditions;
    int end_index;
    bool has_conditions;
} list_comp_t;

static bool is_functionLiteral(parser_t *parser, int index)
{
    tk_type type = parser->tokens[index].type;

    if (type == TK_FUN)
        return true;

    if (type == TK_ID && parser->tokens[index + 1].type == TK_RARROW)
        return true;

    if (type != TK_LPAREN)
        return false;

    int depth = 1;

    for (int i = index + 1; parser->tokens[i].type != TK_EOF; i++)
    {
        if (parser->tokens[i].type == TK_LPAREN)
            depth++;
        else if (parser->tokens[i].type == TK_RPAREN)
            depth--;

        if (depth == 0)
            return parser->tokens[i + 1].type == TK_RARROW;
    }

    return false;
}

static bool is_objectLiteral(parser_t *parser, int index)
{
    return parser->tokens[index].type == TK_LBRACE;
}

static char *get_pendingFunctionName(parser_t *parser)
{
    char *name = parser->fun_name;

    parser->fun_name = NULL;

    return name ? strdup(name) : NULL;
}

static void emit_mapFinalize(parser_t *parser)
{
    int name_index = 0xFF;
    char *descr = "";

    if (parser->object_name != NULL)
    {
        name_index = store_name(parser->comp, parser->object_name);
        descr = parser->object_name;
    }

    emit_8u(parser->comp, OP_MAP_FINALIZE, descr, name_index);
}

static void emit_boundMethodCall(parser_t *parser, const char *receiver, const char *method, int argc)
{
    load_variable(parser->comp, (char *)receiver);

    int method_index = store_const(parser->comp, NEW_OBJ(new_pistring((char *)method)));
    emit_16u(parser->comp, OP_LOAD_CONST, (char *)method, method_index);
    emit(parser->comp, OP_GET_MEMBER);
}

/**
 * Emits the bytecode for a spread list literal.
 * A spread list literal is a list literal with a spread operator (**) at the end.
 * This allows the list to be extended with the elements of another list.
 *
 * @param parser The parser object.
 */
static void emit_spreadListLiteral(parser_t *parser)
{
    emit_16u(parser->comp, OP_PUSH_LIST, "", 0);

    // If the list literal is empty, emit OP_LIST_FINALIZE and return
    if (match(parser, TK_RBRACKET))
    {
        emit(parser->comp, OP_LIST_FINALIZE);
        return;
    }

    // Loop until the end of the list literal or a spread operator is encountered
    do
    {
        // If the end of the list literal is reached, break
        if (check(parser, TK_RBRACKET))
            break;

        // Check if the current token is a spread operator
        bool is_spread = match(parser, TK_ELLIPSIS);

        // Parse the expression after the spread operator if it exists
        cond_expr(parser);

        // Emit the bytecode for extending the list if a spread operator was encountered
        emit(parser->comp, is_spread ? OP_LIST_EXTEND : OP_LIST_APPEND);
    } while (match(parser, TK_COMMA));

    // Consume the end of the list literal
    consume(parser, TK_RBRACKET, "Expect ']' at the end of list literal.");
    // Emit the bytecode for finalizing the list
    emit(parser->comp, OP_LIST_FINALIZE);
}

/**
 * Checks if the current function call has spread arguments.
 * A spread argument is a argument that is prefixed with a spread operator (...)
 * This allows the argument to be passed as a variable number of arguments.
 *
 * @param parser The parser object.
 * @return True if the current function call has spread arguments, false otherwise.
 */
static bool call_hasSpreadArgs(parser_t *parser)
{
    int index = parser->current;
    int paren_depth = 1;   // The parentheses of the function call
    int bracket_depth = 0; // The brackets of a list literal
    int brace_depth = 0;   // The braces of a dictionary literal

    while (parser->tokens[index].type != TK_EOF)
    {
        token_t token = parser->tokens[index++];

        switch (token.type)
        {
        case TK_LPAREN:
            // Increment the parentheses depth
            paren_depth++;
            break;
        case TK_RPAREN:
            // Decrement the parentheses depth
            paren_depth--;
            if (paren_depth == 0)
                // If the parentheses depth is 0, the function call has ended
                return false;
            break;
        case TK_LBRACKET:
            // Increment the brackets depth
            bracket_depth++;
            break;
        case TK_RBRACKET:
            if (bracket_depth > 0)
                // Decrement the brackets depth
                bracket_depth--;
            break;
        case TK_LBRACE:
            // Increment the braces depth
            brace_depth++;
            break;
        case TK_RBRACE:
            if (brace_depth > 0)
                // Decrement the braces depth
                brace_depth--;
            break;
        case TK_ELLIPSIS:
            // If the parentheses depth is 1, the brackets depth is 0, and the braces depth is 0,
            // the current token is a spread operator
            if (paren_depth == 1 && bracket_depth == 0 && brace_depth == 0)
                return true;
            break;
        default:
            break;
        }
    }

    return false;
}

/**
 * Checks if the current token is a spread operator in a list.
 *
 * @param parser The parser state.
 * @return true if the current token is a spread operator, false otherwise.
 */
static bool list_hasSpreadItems(parser_t *parser)
{
    int index = parser->current;
    int paren_depth = 0;   // Depth of parentheses
    int bracket_depth = 1; // Depth of brackets
    int brace_depth = 0;   // Depth of braces

    while (parser->tokens[index].type != TK_EOF)
    {
        token_t token = parser->tokens[index++];

        switch (token.type)
        {
        case TK_LPAREN:
            // Increment the parentheses depth
            paren_depth++;
            break;
        case TK_RPAREN:
            // Decrement the parentheses depth
            if (paren_depth > 0)
                paren_depth--;
            break;
        case TK_LBRACKET:
            // Increment the brackets depth
            bracket_depth++;
            break;
        case TK_RBRACKET:
            // Decrement the brackets depth
            bracket_depth--;
            if (bracket_depth == 0)
                // If the brackets depth is 0, the current token is not a spread operator
                return false;
            break;
        case TK_LBRACE:
            // Increment the braces depth
            brace_depth++;
            break;
        case TK_RBRACE:
            // Decrement the braces depth
            if (brace_depth > 0)
                brace_depth--;
            break;
        case TK_ELLIPSIS:
            // If the parentheses depth is 0, the brackets depth is 1, and the braces depth is 0,
            // the current token is a spread operator
            if (paren_depth == 0 && bracket_depth == 1 && brace_depth == 0)
                return true;
            break;
        default:
            break;
        }
    }

    return false;
}

/**
 * Checks if the current map expression has spread items.
 * A spread item is a value that is prefixed with a spread operator (...).
 * This allows the value to be passed as a variable number of arguments.
 *
 * @param parser The parser object.
 * @return True if the current map expression has spread items, false otherwise.
 */
static bool map_hasSpreadItems(parser_t *parser)
{
    int index = parser->current;
    int paren_depth = 0;   // The parentheses depth of the current expression
    int bracket_depth = 0; // The brackets depth of the current expression
    int brace_depth = 1;   // The braces depth of the current expression

    while (parser->tokens[index].type != TK_EOF)
    {
        token_t token = parser->tokens[index++];

        switch (token.type)
        {
        case TK_LPAREN:
            paren_depth++; // Increment the parentheses depth
            break;
        case TK_RPAREN:
            if (paren_depth > 0)
                paren_depth--; // Decrement the parentheses depth
            break;
        case TK_LBRACKET:
            bracket_depth++; // Increment the brackets depth
            break;
        case TK_RBRACKET:
            if (bracket_depth > 0)
                bracket_depth--; // Decrement the brackets depth
            break;
        case TK_LBRACE:
            brace_depth++; // Increment the braces depth
            break;
        case TK_RBRACE:
            brace_depth--; // Decrement the braces depth
            if (brace_depth == 0)
                return false; // If the braces depth is 0, the current token is not a spread operator
            break;
        case TK_ELLIPSIS:
            if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 1)
                return true; // If the parentheses depth is 0, the brackets depth is 0, and the braces depth is 1, the current token is a spread operator
            break;
        default:
            break;
        }
    }

    return false;
}

static bool is_mapEntry(parser_t *parser)
{
    if (check(parser, TK_ELLIPSIS))
        return true;

    token_t token = peek(parser);
    switch (token.type)
    {
    case TK_STR:
    case TK_ID:
    case TK_NUM:
    case TK_FALSE:
    case TK_TRUE:
    {
        token_t next_token = parser->tokens[parser->current + 1];
        return next_token.type == TK_COLON || next_token.type == TK_LPAREN;
    }
    default:
        return false;
    }
}

static void emit_setLiteral(parser_t *parser)
{
    int size = 0;
    if (check(parser, TK_RBRACE))
    {
        consume(parser, TK_RBRACE, "Expect '}' at the end of set literal.");
        emit_16u(parser->comp, OP_PUSH_SET, "", 0);
        return;
    }

    do
    {
        if (check(parser, TK_RBRACE))
            break;

        cond_expr(parser);
        size++;
    } while (match(parser, TK_COMMA));

    consume(parser, TK_RBRACE, "Expect '}' at the end of set literal.");
    emit_16u(parser->comp, OP_PUSH_SET, "", size);
}

/**
 * Scans the current list comprehension expression and stores its parts in the given structure.
 *
 * @param parser The parser object.
 * @param comp The list comprehension structure to store the parts in.
 * @return True if the list comprehension expression has been successfully scanned, false otherwise.
 */
static bool scan_listComprehension(parser_t *parser, list_comp_t *comp)
{
    int index = parser->current;
    int paren_depth = 0;   // The parentheses depth of the current expression
    int bracket_depth = 1; // The brackets depth of the current expression
    int brace_depth = 0;   // The braces depth of the current expression
    int ternary_depth = 0; // The ternary operator depth of the current expression
    int first_colon = -1;  // The index of the first colon in the current expression
    int second_colon = -1; // The index of the second colon in the current expression

    while (parser->tokens[index].type != TK_EOF)
    {
        token_t token = parser->tokens[index];

        switch (token.type)
        {
        case TK_LPAREN:
            // Increment the parentheses depth
            paren_depth++;
            break;
        case TK_RPAREN:
            // Decrement the parentheses depth
            if (paren_depth > 0)
                paren_depth--;
            break;
        case TK_LBRACKET:
            // Increment the brackets depth
            bracket_depth++;
            break;
        case TK_RBRACKET:
            // Decrement the brackets depth
            bracket_depth--;
            if (bracket_depth == 0)
            {
                // If the brackets depth is 0, the current token is not a spread operator
                if (first_colon == -1)
                    return false;

                // Store the parts of the list comprehension expression in the given structure
                comp->result.start = parser->current;
                comp->result.end = first_colon;
                comp->iterators.start = first_colon + 1;
                comp->iterators.end = (second_colon == -1) ? index : second_colon;
                comp->conditions.start = (second_colon == -1) ? index : second_colon + 1;
                comp->conditions.end = index;
                comp->has_conditions = second_colon != -1;
                comp->end_index = index;
                return true;
            }
            break;
        case TK_LBRACE:
            // Increment the braces depth
            brace_depth++;
            break;
        case TK_RBRACE:
            // Decrement the braces depth
            if (brace_depth > 0)
                brace_depth--;
            break;
        case TK_QUESTION:
            // Increment the ternary operator depth
            if (paren_depth == 0 && bracket_depth == 1 && brace_depth == 0)
                ternary_depth++;
            break;
        case TK_COLON:
            // If the parentheses depth is 0, the brackets depth is 1, and the braces depth is 0
            if (paren_depth == 0 && bracket_depth == 1 && brace_depth == 0)
            {
                // If the ternary operator depth is greater than 0
                if (ternary_depth > 0)
                    ternary_depth--;
                else if (first_colon == -1)
                    first_colon = index;
                else if (second_colon == -1)
                    second_colon = index;
                else
                    p_errorf(token.line, token.column,
                             "List comprehensions support at most one iterator clause and one condition clause separator.");
            }
            break;
        default:
            break;
        }

        index++;
    }

    return false;
}

/**
 * Checks if the current token is the start of a list comprehension.
 * If it is, parses the list comprehension and returns true. Otherwise, returns false.
 *
 * @param parser The current parser instance.
 * @return true if the current token is the start of a list comprehension, false otherwise.
 */
static bool list_isComprehension(parser_t *parser)
{
    // The list comprehension structure to parse
    list_comp_t comp;

    // Check if the current token is the start of a list comprehension
    // If it is, parse the list comprehension
    return scan_listComprehension(parser, &comp);
}

/**
 * Compiles an expression segment. If the segment is empty, an error is raised.
 *
 * @param parser The parser object.
 * @param segment The segment of tokens to compile.
 * @param message The error message to display if the segment is empty.
 */
static void compile_segmentExpr(parser_t *parser, segment_t segment, const char *message)
{
    // Check if the segment is empty
    if (segment.start >= segment.end)
    {
        // If it is, raise an error with the given message
        token_t token = parser->tokens[segment.start];
        p_error(message, token.line, token.column);
    }

    // Save the current position of the parser
    int saved = parser->current;

    // Save the token at the end of the segment
    token_t saved_end = parser->tokens[segment.end];

    // Set the current position of the parser to the start of the segment
    parser->current = segment.start;

    // Set the type of the token at the end of the segment to TK_EOF
    parser->tokens[segment.end].type = TK_EOF;

    // Compile the expression
    cond_expr(parser);

    // Restore the token at the end of the segment
    parser->tokens[segment.end] = saved_end;

    // Check if the parser has reached the end of the segment
    if (parser->current != segment.end)
    {
        // If not, raise an error
        token_t token = parser->tokens[parser->current];
        p_error(message, token.line, token.column);
    }

    // Restore the current position of the parser
    parser->current = saved;
}

/**
 * Parses the list comprehension iterator expressions.
 *
 * @param parser The current parser instance.
 * @param segment The segment of tokens to parse.
 * @param iters The iterator expressions to store.
 * @param max_iters The maximum number of iterator expressions to store.
 * @return The number of iterator expressions parsed.
 */
static int parse_compIterators(parser_t *parser, segment_t segment, comp_iter_t *iters, int max_iters)
{
    int index = segment.start;
    int count = 0;

    while (index < segment.end)
    {
        // Check for too many iterators
        if (count >= max_iters)
        {
            token_t token = parser->tokens[index];
            p_errorf(token.line, token.column, "Too many iterators in list comprehension.");
        }

        // Parse the iterator variable name
        token_t name = parser->tokens[index++];
        if (name.type != TK_ID)
            p_errorf(name.line, name.column, "Expect iterator variable name in list comprehension.");

        // Check for the 'in' keyword
        if (index >= segment.end || parser->tokens[index].type != TK_IN)
        {
            token_t token = parser->tokens[index < segment.end ? index : segment.end - 1];
            p_errorf(token.line, token.column, "Expect 'in' after iterator variable in list comprehension.");
        }
        index++;

        // Parse the iterable expression
        int expr_start = index;
        int paren_depth = 0;
        int bracket_depth = 0;
        int brace_depth = 0;

        while (index < segment.end)
        {
            token_t token = parser->tokens[index];
            if (token.type == TK_COMMA && paren_depth == 0 && bracket_depth == 0 && brace_depth == 0)
                break;

            switch (token.type)
            {
            case TK_LPAREN:
                paren_depth++;
                break;
            case TK_RPAREN:
                if (paren_depth > 0)
                    paren_depth--;
                break;
            case TK_LBRACKET:
                bracket_depth++;
                break;
            case TK_RBRACKET:
                if (bracket_depth > 0)
                    bracket_depth--;
                break;
            case TK_LBRACE:
                brace_depth++;
                break;
            case TK_RBRACE:
                if (brace_depth > 0)
                    brace_depth--;
                break;
            default:
                break;
            }

            index++;
        }

        // Check for empty iterable expression
        if (expr_start == index)
            p_errorf(name.line, name.column, "Expect iterable expression after 'in' in list comprehension.");

        // Store the iterator expression
        iters[count].name = name;
        iters[count].iterable.start = expr_start;
        iters[count].iterable.end = index;
        count++;

        // Check for trailing comma
        if (index < segment.end && parser->tokens[index].type == TK_COMMA)
            index++;
    }

    return count;
}

/**
 * Parses the conditions of a list comprehension expression.
 * List comprehensions are a concise way of creating lists from other
 * iterables. They are defined as [expr for var in iterable if cond1, cond2, ...],
 * where expr is an expression that is evaluated for each value of var in
 * iterable, and cond1, cond2, ... are conditions that must be true for
 * the value to be included in the resulting list.
 *
 * This function parses the conditions of a list comprehension expression.
 * It takes a parser object, a segment of tokens that contains the
 * conditions, and an array of segment_t objects that will be filled with the
 * parsed conditions. The function returns the number of conditions parsed.
 *
 * @param parser The parser object.
 * @param segment The segment of tokens that contains the conditions.
 * @param conds The array of segment_t objects that will be filled with the parsed conditions.
 * @param max_conds The maximum number of conditions that can be parsed.
 * @return The number of conditions parsed.
 */
static int parse_compConditions(parser_t *parser, segment_t segment, segment_t *conds, int max_conds)
{
    if (segment.start >= segment.end)
        return 0;

    int index = segment.start;
    int count = 0;

    // Loop until we reach the end of the segment
    while (index < segment.end)
    {
        // Check if we have reached the maximum number of conditions
        if (count >= max_conds)
        {
            token_t token = parser->tokens[index];
            p_errorf(token.line, token.column, "Too many conditions in list comprehension.");
        }

        int expr_start = index;
        int paren_depth = 0;
        int bracket_depth = 0;
        int brace_depth = 0;

        // Loop until we reach the end of the expression or a comma
        while (index < segment.end)
        {
            token_t token = parser->tokens[index];
            if (token.type == TK_COMMA && paren_depth == 0 && bracket_depth == 0 && brace_depth == 0)
                break;

            switch (token.type)
            {
            case TK_LPAREN:
                paren_depth++; // Increment the parentheses depth
                break;
            case TK_RPAREN:
                if (paren_depth > 0)
                    paren_depth--; // Decrement the parentheses depth
                break;
            case TK_LBRACKET:
                bracket_depth++; // Increment the brackets depth
                break;
            case TK_RBRACKET:
                if (bracket_depth > 0)
                    bracket_depth--; // Decrement the brackets depth
                break;
            case TK_LBRACE:
                brace_depth++; // Increment the braces depth
                break;
            case TK_RBRACE:
                if (brace_depth > 0)
                    brace_depth--; // Decrement the braces depth
                break;
            default:
                break;
            }

            index++;
        }

        conds[count].start = expr_start;
        conds[count].end = index;
        count++;

        // Check if there is a trailing comma
        if (index < segment.end && parser->tokens[index].type == TK_COMMA)
            index++;
    }

    return count;
}

/**
 * Emits bytecode for the loop of a list comprehension expression.
 * The loop iterates over each iterator expression in the list comprehension,
 * and for each iterator expression, it compiles the expression and checks
 * if the resulting value is true. If it is, it compiles the list comprehension
 * expression and appends the resulting value to the list.
 *
 * @param parser The parser object.
 * @param comp The list comprehension expression to emit bytecode for.
 * @param iters The array of iterator expressions.
 * @param iter_count The number of iterator expressions.
 * @param conds The array of condition expressions.
 * @param cond_count The number of condition expressions.
 * @param iter_index The index of the current iterator expression.
 * @param acc_slot The slot in the local variable table where the resulting list is stored.
 */
static void emit_listCompLoops(parser_t *parser, list_comp_t *comp,
                               comp_iter_t *iters, int iter_count,
                               segment_t *conds, int cond_count,
                               int iter_index, int acc_slot)
{
    if (iter_index == iter_count)
    {
        // If we have reached the end of the iterator expressions, compile the condition
        // expressions and list comprehension expression, and append the resulting value to the list.
        int jumps[32];
        int jump_count = 0;

        for (int i = 0; i < cond_count; i++)
        {
            // Compile the condition expression at conds[i]
            compile_segmentExpr(parser, conds[i], "Invalid list comprehension condition.");
            // Emit a jump if false instruction to jump over the list comprehension expression if the condition is false
            jumps[jump_count++] = emit_16u(parser->comp, OP_JUMP_IF_FALSE, "", 0);
        }

        // Compile the list comprehension expression
        compile_segmentExpr(parser, comp->result, "Invalid list comprehension expression.");
        // Emit an instruction to append the resulting value to the list
        emit_8u(parser->comp, OP_COMP_APPEND, "<comp>", acc_slot);

        // Patch the jump if false instructions to jump over the list comprehension expression if the condition is false
        for (int i = 0; i < jump_count; i++)
            patch_jump(parser->comp, jumps[i]);
        return;
    }

    // Get the current iterator expression
    token_t iter_token = iters[iter_index].name;

    // Set the position of the parser to the start of the current iterator expression
    set_pos(parser, parser->tokens[iters[iter_index].iterable.start]);

    // Compile the current iterator expression
    compile_segmentExpr(parser, iters[iter_index].iterable, "Invalid list comprehension iterator expression.");
    // Emit an instruction to push the iterator expression onto the stack
    emit(parser->comp, OP_PUSH_ITER);

    // Set the position of the parser to the current iterator expression
    set_pos(parser, iter_token);

    // Emit an instruction to start the loop
    int address = emit_16u(parser->comp, OP_LOOP, "", 0);

    // Push a new scope onto the stack
    push_scope(parser->comp);
    // Add a new local variable to the scope with the name of the current iterator expression
    add_variable(parser->comp, token_value(iter_token));
    // Push a new loop onto the stack
    push_loop(parser->comp, address - 2, true);

    // Recursively emit bytecode for the loop
    emit_listCompLoops(parser, comp, iters, iter_count, conds, cond_count, iter_index + 1, acc_slot);

    // Pop the scope off the stack
    pop_scope(parser->comp);
    // Pop the loop off the stack
    pop_loop(parser->comp, address - 2);
    // Patch the jump if false instruction to jump over the list comprehension expression if the condition is false
    patch_jump(parser->comp, address);
}

/**
 * Emits the bytecode for a list comprehension expression.
 * List comprehension expressions are in the form of [x for x in y if z].
 * This function parses the list comprehension expression and emits the appropriate bytecode.
 * @param parser The parser object.
 */
static void emit_listComprehension(parser_t *parser)
{
    list_comp_t comp;
    // Parse the list comprehension expression
    if (!scan_listComprehension(parser, &comp))
        return;

    comp_iter_t iters[16];
    segment_t conds[32];
    int iter_count = parse_compIterators(parser, comp.iterators, iters, 16);
    int cond_count = comp.has_conditions ? parse_compConditions(parser, comp.conditions, conds, 32) : 0;

    if (iter_count == 0)
    {
        token_t token = parser->tokens[comp.iterators.start];
        p_errorf(token.line, token.column, "List comprehension requires at least one iterator.");
    }

    // Create a hidden variable name for the list comprehension result
    char hidden_name[32];
    snprintf(hidden_name, sizeof(hidden_name), "<comp_%d>", comp.result.start);

    // Emit an instruction to push a new list onto the stack
    emit_16u(parser->comp, OP_PUSH_LIST, "", 0);
    // Add a new local variable to the scope with the hidden variable name
    add_local(parser->comp, hidden_name);
    int acc_slot = get_local(parser->comp, hidden_name);

    // Recursively emit bytecode for the loop
    emit_listCompLoops(parser, &comp, iters, iter_count, conds, cond_count, 0, acc_slot);

    // Emit an instruction to finalize the list
    emit(parser->comp, OP_LIST_FINALIZE);
    // Remove the local variable from the scope
    remove_locals(parser->comp, 1);
    // Set the position of the parser to the end of the list comprehension
    parser->current = comp.end_index;
    // Consume the end of the list comprehension
    consume(parser, TK_RBRACKET, "Expect ']' after list comprehension.");
}

/**
 * Peeks at the current token from the tokens array.
 * This function is used to inspect the current token without advancing the parser.
 * @return the current token
 */
static token_t peek(parser_t *parser)
{
    return parser->tokens[parser->current];
}

/**
 * Peeks at the next token from the tokens array.
 * This function is used to inspect the token immediately following the current token
 * without advancing the parser.
 * @return the next token
 */

static token_t peek_next(parser_t *parser)
{
    return parser->tokens[parser->current + 1];
}

/**
 * Checks if the parser is at the end of the token stream.
 * @return true if the parser is at the end of the token stream, false otherwise
 */
static bool is_atEnd(parser_t *parser)
{
    return peek(parser).type == TK_EOF;
}

/**
 * Retrieves the previous token from the tokens array.
 *
 * This function is used to access the token that was previously processed
 * by the parser. It is used to handle certain language constructs that
 * require access to the previous token.
 *
 * @return the previous token
 */
static token_t previous(parser_t *parser)
{
    return parser->tokens[parser->current - 1];
}

/**
 * Checks if a given token is a delimiter.
 * A delimiter is a token that delimits (or separates) other tokens. In this
 * case, the only delimiter is the semicolon (;).
 * @param token the token to check
 * @return true if the token is a delimiter, false otherwise
 */
static bool is_delimiter(parser_t *parser, token_t token)
{
    return token.type == TK_SEMICOLON;
}

/**
 * Advances the parser to the next token and returns the previous token.
 * @return the previous token
 */
static token_t next(parser_t *parser)
{
    if (!is_atEnd(parser))
    {
        parser->current++;

        token_t tok = peek(parser);
        if (!is_delimiter(parser, tok))
            parser->last = tok;
    }
    return previous(parser);
}

/**
 * Checks if the current token matches a given type.
 * @param type the type of token to check
 * @return true if the current token matches the given type, false otherwise
 */
static bool check(parser_t *parser, tk_type type)
{
    return !is_atEnd(parser) && peek(parser).type == type;
}

/**
 * Checks if the current token matches one of the given types and
 * advances the parser if a match is found.
 * @param t_count the number of types to check
 * @param ... the types to check
 * @return true if the current token matches one of the given types, false otherwise
 */
static bool match_n(parser_t *parser, int t_count, ...)
{
    va_list args;
    va_start(args, t_count);

    for (int i = 0; i < t_count; i++)
    {
        tk_type type = va_arg(args, tk_type);
        if (check(parser, type))
        {
            next(parser);
            va_end(args);
            return true;
        }
    }
    va_end(args);
    return false;
}

/**
 * Checks if the current token matches a given type and advances the parser
 * if a match is found.
 * @param type the type of token to check
 * @return true if the current token matches the given type and has been
 *         advanced, false otherwise
 */
static bool match(parser_t *parser, tk_type type)
{
    if (check(parser, type))
    {
        next(parser);
        return true;
    }
    return false;
}

/**
 * Checks if the current token is the start of a destructure assignment.
 * If so, advances the parser until the end of the assignment.
 * @param parser The parser object.
 * @return true if the current token is the start of a destructure assignment, false otherwise
 */
static bool is_destructure_assign(parser_t *parser)
{
    // Check if the current token is the start of a destructure assignment
    if (!check(parser, TK_LBRACKET))
        return false;

    int current = parser->current;
    next(parser);

    // Check if the current token is the end of a destructure assignment
    if (check(parser, TK_RBRACKET))
    {
        parser->current = current;
        return false;
    }

    // Iterate until the end of the assignment
    while (true)
    {
        // Check if the current token is an identifier
        if (!check(parser, TK_ID))
        {
            parser->current = current;
            return false;
        }
        next(parser);

        // Check if the current token is a comma
        if (!match(parser, TK_COMMA))
            break;
    }

    // Check if the current token is an assignment operator
    bool is_assign = match(parser, TK_RBRACKET) && check(parser, TK_ASSIGN);
    parser->current = current;
    return is_assign;
}

/**
 * Checks if the current token matches any of the given types.
 *
 * @param t_count the number of types to check
 * @param ... the types to check
 * @return true if the current token matches one of the given types, false otherwise
 */
static bool check_n(parser_t *parser, int t_count, ...)
{
    if (is_atEnd(parser))
        return false;
    va_list args;
    va_start(args, t_count);
    for (int i = 0; i < t_count; i++)
    {
        tk_type type = va_arg(args, tk_type);
        if (peek(parser).type == type)
        {
            va_end(args);
            return true;
        }
    }
    va_end(args);
    return false;
}

/**
 * Consumes the current token if it matches the given type and advances the
 * parser to the next token.
 *
 * @param type the type of token to check
 * @param message the error message to display if the token does not match
 * @return the consumed token if it matches the given type, or else exit with
 *         an error message
 */
static token_t consume(parser_t *parser, tk_type type, const char *message)
{
    if (check(parser, type))
    {
        // If the token matches the given type, advance the parser and return
        // the consumed token
        token_t token = next(parser);
        return token;
    }
    else if (message != NULL)
        // If there is an error message, print it to the standard error stream
        // with the line and column of the error
        p_error(message, peek(parser).line, peek(parser).column);
    else
        // If there is no error message, use a default error message
        p_error("Unexpected token", peek(parser).line, peek(parser).column);

    if (global_errorHandler)
    {
        token_t token = peek(parser);

        while (!is_atEnd(parser))
            advance(parser);

        return token;
    }

    exit(EXIT_FAILURE);
}

/**
 * Advances the parser to the next token.
 *
 * This function increments the current position of the parser
 * to point to the next token, if the end of the token stream
 * has not been reached.
 */
static void advance(parser_t *parser)
{
    if (!is_atEnd(parser)) // Check if there are more tokens to process
        parser->current++; // Move to the next token
}

/**
 * Advances the parser by a given number of steps. This is used to skip over
 * tokens that are not of interest when parsing.
 * @param steps the number of steps to advance the parser
 **/
static void skip(parser_t *parser, int steps)
{
    parser->current += steps;
}

/**
 * Consumes tokens if they exist in the given types.
 *
 * This function checks whether the current token matches any of the given
 * types and advances the parser if a match is found. It continues to consume
 * tokens as long as they match one of the specified types.
 *
 * @param t_count The number of token types to check against.
 * @param ... Variadic arguments representing the token types to match.
 * @return true if any tokens were consumed, false otherwise.
 */
static bool consume_ifExist(parser_t *parser, int t_count, ...)
{
    bool consumed = false;
    va_list args;
    va_start(args, t_count);

    while (true)
    {
        bool matched = false;

        // Iterate over each token type to check for a match
        for (int i = 0; i < t_count; ++i)
        {
            tk_type type = va_arg(args, tk_type);
            if (check(parser, type))
            {
                advance(parser); // Advance parser if a match is found
                consumed = true;
                matched = true;
                break;
            }
        }

        if (!matched)
            break; // Exit loop if no types matched

        // Reset the va_list to begin checking from the start again
        va_end(args);
        va_start(args, t_count);
    }

    va_end(args);
    return consumed;
}

/**
 * Updates the parser's current position to match the given token's position.
 *
 * This function sets the current line and column of the parser's compiler
 * to the line and column positions of the provided token.
 *
 * @param parser The parser whose position is to be updated.
 * @param token The token whose position is used to update the parser's position.
 */
void set_pos(parser_t *parser, token_t token)
{
    // Set the current line of the compiler to the token's line
    parser->comp->current_line = token.line;

    // Set the current column of the compiler to the token's column
    parser->comp->current_col = token.column;
}

/**
 * Checks if there is a line break between the previous and current token.
 *
 * This function compares the line numbers of the previous token and the
 * current token to determine if there is a line break between them.
 *
 * @return true if there is a line break, false otherwise.
 */
static bool is_lineBreak(parser_t *parser)
{
    // Compare line numbers of previous and current tokens
    return previous(parser).line < peek(parser).line || peek(parser).type == TK_EOF;
}

bool need_delimiter(parser_t *parser)
{
    // If there's no explicit semicolon,
    // and it's not a line break,
    // and the next token is not a closing brace,
    // then we should insert a semicolon.
    if (!consume_ifExist(parser, 1, TK_SEMICOLON))
    {
        if (!is_lineBreak(parser))
        {
            if (!check(parser, TK_RBRACE))
                return true;
        }
    }

    // If we get here, we don't need a delimiter
    return false;
}
/**
 * Checks if the current token is an assignment operator.
 * The function verifies if the parser is in a store state and if the current
 * token matches any of the assignment operators. If so, it resets the store
 * state and returns true.
 *
 * @return true if the current token is an assignment operator, false otherwise
 */
static bool is_assign(parser_t *parser)
{
    // Check if the parser is in a store state and the current token is an assignment operator
    if (parser->is_store &&
        (parser->force_store ||
         check_n(parser, 11, TK_ASSIGN, TK_PLUS_ASSIGN, TK_MINUS_ASSIGN, TK_DIV_ASSIGN, TK_MULT_ASSIGN,
                 TK_MOD_ASSIGN, TK_BITOR_ASSIGN, TK_XOR_ASSIGN, TK_BITAND_ASSIGN, TK_INCR, TK_DECR)))
    {
        parser->is_store = false; // Reset the store state
        parser->force_store = false;
        return true; // Return true as the token is an assignment operator
    }

    return false; // Return false if no assignment operator is found
}

static bool has_accessContinuation(parser_t *parser, token_t token)
{
    return peek(parser).line == token.line &&
           check_n(parser, 3, TK_DOT, TK_LBRACKET, TK_LPAREN);
}

/**
 * Marks a range of tokens as skipped tokens.
 *
 * This function iterates over a range of tokens and sets the skip flag
 * to true for each of them. This is used to skip over tokens that are not
 * of interest when parsing.
 *
 * @param start the starting index of the range of tokens to be marked
 * @param end the ending index of the range of tokens to be marked
 */
void mark_tokens(parser_t *parser, int start, int end)
{
    // Iterate over the range of tokens and mark them as skipped
    for (int i = start; i < end; i++)
        parser->tokens[i].skip = true;
}

static void skip_letDecl(parser_t *parser)
{
    int depth = 0;

    while (!is_atEnd(parser))
    {
        token_t tok = peek(parser);

        switch (tok.type)
        {
        case TK_LPAREN:
        case TK_LBRACKET:
        case TK_LBRACE:
            depth++;
            break;
        case TK_RPAREN:
        case TK_RBRACKET:
        case TK_RBRACE:
            if (depth > 0)
                depth--;
            break;
        default:
            break;
        }

        if (depth == 0 && (tok.type == TK_SEMICOLON || is_lineBreak(parser)))
        {
            if (tok.type == TK_SEMICOLON)
                next(parser);
            break;
        }

        next(parser);
    }
}

/**
 * Initializes the parser with the provided tokens.
 * Allocates memory for the parser structure and sets default values
 * for various parser state variables.
 *
 * @param tokens the array of tokens to be parsed
 */
parser_t *init_parser(compiler_t *comp, token_t *tokens, ParserMode mode)
{
    // Allocate memory for the parser structure
    parser_t *parser = (parser_t *)malloc(sizeof(parser_t));

    // Initialize the parser's tokens with the provided tokens
    parser->tokens = tokens;

    // Set initial states for parser flags
    parser->access = false;   // Indicates whether access is allowed
    parser->current = 0;      // Start at the first token
    parser->is_store = false; // Store flag set to false
    parser->force_store = false;
    parser->is_return = false;
    parser->has_walrus = false;
    parser->object_member = false;
    parser->fun_name = NULL;
    parser->object_name = NULL;

    // Initialize the compiler associated with the parser
    parser->comp = comp;
    set_errorSource(comp ? comp->source_name : NULL);

    // Set the parsing mode
    parser->mode = mode;
    if (mode == MODE_REPL)
        parser->comp->is_repl = true;

    return parser;
}

/**
 * Parses the provided tokens according to the language's grammar rules.
 * Generates bytecode by calling the emit() function from the compiler module.
 * @param parser the parser structure containing the tokens to be parsed
 */
void parse(parser_t *parser)
{
    if (parser->mode == MODE_REPL)
    {
        // In REPL mode, parse only a single expression statement.
        if (!is_atEnd(parser))
            expr_state(parser);
    }
    else
    {
        // In file mode, parse the entire program.
        program(parser);
    }

    // Emit HALT bytecode to indicate the end of the program
    emit(parser->comp, OP_HALT);

    // Runtime diagnostics need the global instruction metadata even when
    // debug disassembly is disabled.
    ht_put(parser->comp->instrs, "<global>", parser->comp->current->instrs);
}

/**
 * Parses all declarations within the program.
 * This function performs two passes over the tokens:
 * 1. Hoists functions and collects globals.
 * 2. Parses remaining statements while skipping processed tokens.
 *
 * @param parser The parser structure containing the tokens to be parsed.
 */
static void declarations(parser_t *parser)
{
    int depth = 0;

    // First pass: Hoist functions and collect globals
    while (!is_atEnd(parser))
    {
        // Track block depth to ignore inner declarations
        if (check(parser, TK_LBRACE))
            depth++;
        else if (check(parser, TK_RBRACE))
            depth--;

        // Skip inner block declarations
        if (depth > 0)
        {
            next(parser);
            continue;
        }

        // Hoist function declarations
        if (match(parser, TK_FUN) && !match(parser, TK_LPAREN))
        {
            int start = parser->current - 1; // Start at 'fun'
            func_decl(parser);               // Parse and hoist function
            int end = parser->current;
            mark_tokens(parser, start, end); // Mark tokens as processed
        }
        // Skip global variable declarations to preserve execution order
        else if (match(parser, TK_LET))
            skip_letDecl(parser);
        else
            next(parser); // Move to the next token
    }

    // Reset parser position for second pass
    parser->current = 0;

    // Second pass: Parse remaining code (skipping processed tokens)
    while (!is_atEnd(parser))
    {
        // Skip tokens marked as processed
        if (parser->tokens[parser->current].skip)
            next(parser);
        else
            // statement(parser); // Parse remaining statements
            declaration(parser); // Parse remaining declarations/statements in order
    }
}

/**
 * Program -> Declaration* EOF
 * Parses the entire program consisting of declarations and a terminating EOF.
 */
static void program(parser_t *parser)
{
    // Parse the program
    declarations(parser);
}

/**
 * Parses a declaration, which can be either a variable declaration,
 * a function declaration, or a statement.
 * Declaration -> VarDecl | FunDecl | Statement
 * @param parser The parser structure containing the tokens to be parsed.
 */
static void declaration(parser_t *parser)
{
    // Check if the declaration is a variable declaration using 'let'
    if (match(parser, TK_LET))
        var_decl(parser); // Parse the variable declaration
    // Check if the declaration is a function declaration using 'fun'
    else if (match(parser, TK_FUN))
        func_decl(parser);
    else if (match(parser, TK_CLASS))
        class_decl(parser);
    // If not a variable or function declaration, parse as a statement
    else
    {
        statement(parser); // Parse as a statement
        parser->is_return = false;
    }
}

/**
 * var_decl -> "let" IDENT EQUAL expr
 * A variable declaration is a statement that declares a variable.
 */
static void var_decl(parser_t *parser)
{
    do
    {
        variable(parser);
    } while (match(parser, TK_COMMA));
    consume_ifExist(parser, 1, TK_SEMICOLON);
}

/**
 * variable -> IDENT
 * A variable is a name that can be used to refer to a value.
 * It is used to parse a variable in a variable declaration.
 */
static void variable(parser_t *parser)
{
    int index = -1;

    // Parse the variable name
    token_t token = consume(parser, TK_ID, "Expect variable name");
    char *name = token_value(token);

    // parser->comp->name = strdup(name); // Store the variable name;

    // Check if the variable is being assigned a value
    if (match(parser, TK_ASSIGN))
    {
        char *prev_fun = parser->fun_name;    // Store the pending function name
        char *prev_obj = parser->object_name; // Store the pending object name

        if (is_functionLiteral(parser, parser->current))
            parser->fun_name = name;
        if (is_objectLiteral(parser, parser->current))
            parser->object_name = name;

        assignment(parser, true);
        parser->fun_name = prev_fun;
        parser->object_name = prev_obj;
    }

    else
        emit(parser->comp, OP_PUSH_NIL);

    // Store the variable
    add_variable(parser->comp, name);
}

/**
 * param_list -> IDENT ( COMMA IDENT )* ( COMMA )?
 *
 * param_list parses a parameter list for a function declaration.
 * It returns a list of strings, where each string is the name of a parameter.
 * @returns a list of strings, where each string is the name of a parameter
 */
static list_t *param_list(parser_t *parser)
{
    int size = 0;
    token_t name;
    list_t *params = list_create(sizeof(String));

    set_pos(parser, previous(parser));

    // parse the parameter list until the right parenthesis is encountered
    if (!check(parser, TK_RPAREN))
    {
        do
        {
            if (size >= 32)
                p_error("Can't have more than 32 parameters.", peek(parser).line, peek(parser).column);

            // parse the parameter name
            name = consume(parser, TK_ID, "Expect parameter name.");
            list_add(params, new_string(token_value(name)));

            // parse the default value if it is present
            if (match(parser, TK_ASSIGN))
                expr(parser);
            else
                emit(parser->comp, OP_PUSH_NIL);

            // increment the parameter counter
            size++;

            // continue parsing the parameter list if there is a comma
        } while (match(parser, TK_COMMA));
    }

    return params;
}

static void emit_spreadMapLiteral(parser_t *parser)
{
    emit_16u(parser->comp, OP_PUSH_MAP, "", 0);

    if (match(parser, TK_RBRACE))
    {
        emit_mapFinalize(parser);
        return;
    }

    do
    {
        if (check(parser, TK_RBRACE))
            break;

        if (match(parser, TK_ELLIPSIS))
        {
            cond_expr(parser);
            emit(parser->comp, OP_MAP_EXTEND);
            continue;
        }

        char *key;
        int index = 0;

        if (match_n(parser, 5, TK_STR, TK_ID, TK_NUM, TK_FALSE, TK_TRUE))
        {
            key = tk_string(previous(parser));
            index = store_const(parser->comp, NEW_OBJ(new_pistring(key)));
        }
        else
            p_error("Unexpected key expression.", peek(parser).line, peek(parser).column);

        if (match(parser, TK_LPAREN))
        {
            list_t *params = param_list(parser);
            int size = list_size(params);
            consume(parser, TK_RPAREN, "Expect ')' before function body.");
            consume(parser, TK_LBRACE, "Expect '{' before function body.");

            push_function(parser->comp, key);
            parser->comp->current->param_names = params;

            if (is_object(parser->comp))
                add_local(parser->comp, "this");

            for (int i = 0; i < size; i++)
                add_local(parser->comp, string_get(params, i));
            add_local(parser->comp, "args");
            add_local(parser->comp, "kw_args");

            if (match(parser, TK_RBRACE))
            {
                if (is_constructor(parser->comp))
                    emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                else
                    emit(parser->comp, OP_PUSH_NIL);
                emit(parser->comp, OP_RETURN);
            }
            else
            {
                while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
                    declaration(parser);

                if (!parser->is_return)
                {
                    if (is_constructor(parser->comp))
                        emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                    else
                        emit(parser->comp, OP_PUSH_NIL);
                    emit(parser->comp, OP_RETURN);

                    parser->is_return = false;
                }
            }

            pop_function(parser->comp, size);
            consume(parser, TK_RBRACE, "Expect '}' after function body.");
        }
        else
        {
            if (strcmp(key, "constructor") == 0)
                p_error("Constructor is a reserved keyword.", peek(parser).line, peek(parser).column);
            consume(parser, TK_COLON, "Expect ':' after object key expression.");
            char *prev_obj = parser->object_name;
            parser->object_name = NULL;
            cond_expr(parser);
            parser->object_name = prev_obj;
        }

        emit_16u(parser->comp, OP_LOAD_CONST, key, index);
        emit(parser->comp, OP_MAP_SET);
    } while (match(parser, TK_COMMA) && !check(parser, TK_RBRACE));

    consume(parser, TK_RBRACE, "Expect '}' at the end of map literal.");
    emit_mapFinalize(parser);
}

static void emit_classMap(parser_t *parser, const char *class_name)
{
    char *prev_obj = parser->object_name;
    parser->object_name = (char *)class_name;

    push_object(parser->comp);

    int size = 0;
    while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
    {
        token_t key_tok = consume(parser, TK_ID, "Expect class member name.");
        char *key = token_value(key_tok);
        int index = store_const(parser->comp, NEW_OBJ(new_pistring(key)));

        if (match(parser, TK_LPAREN))
        {
            list_t *params = param_list(parser);
            int param_count = list_size(params);
            consume(parser, TK_RPAREN, "Expect ')' before method body.");
            consume(parser, TK_LBRACE, "Expect '{' before method body.");

            push_function(parser->comp, key);
            parser->comp->current->param_names = params;

            add_local(parser->comp, "this");

            for (int i = 0; i < param_count; i++)
                add_local(parser->comp, string_get(params, i));
            add_local(parser->comp, "args");
            add_local(parser->comp, "kw_args");

            if (match(parser, TK_RBRACE))
            {
                if (is_constructor(parser->comp))
                    emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                else
                    emit(parser->comp, OP_PUSH_NIL);
                emit(parser->comp, OP_RETURN);
            }
            else
            {
                while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
                    declaration(parser);

                if (!parser->is_return)
                {
                    if (is_constructor(parser->comp))
                        emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                    else
                        emit(parser->comp, OP_PUSH_NIL);
                    emit(parser->comp, OP_RETURN);
                }
            }

            parser->is_return = false;
            pop_function(parser->comp, param_count);
            consume(parser, TK_RBRACE, "Expect '}' after method body.");
        }
        else
        {
            if (strcmp(key, "constructor") == 0)
                p_error("Constructor is a reserved keyword.", peek(parser).line, peek(parser).column);

            consume(parser, TK_ASSIGN, "Expect '=' after static class member name.");

            bool prev_object_member = parser->object_member;
            char *prev_fun = parser->fun_name;
            char *member_prev_obj = parser->object_name;
            parser->object_member = true;
            parser->object_name = NULL;

            if (is_functionLiteral(parser, parser->current))
                parser->fun_name = key;

            cond_expr(parser);
            parser->object_member = prev_object_member;
            parser->fun_name = prev_fun;
            parser->object_name = member_prev_obj;
        }

        emit_16u(parser->comp, OP_LOAD_CONST, key, index);
        size++;

        bool had_comma = consume_ifExist(parser, 1, TK_COMMA);
        if (!had_comma && need_delimiter(parser))
            p_error("Expected delimiter between class members.", peek(parser).line, peek(parser).column);
    }

    consume(parser, TK_RBRACE, "Expect '}' after class body.");
    pop_object(parser->comp);
    emit_16u(parser->comp, OP_PUSH_MAP, "", size);
    emit_mapFinalize(parser);

    parser->object_name = prev_obj;
}

static void class_decl(parser_t *parser)
{
    token_t name_tok = consume(parser, TK_ID, "Expect class name.");
    char *class_name = token_value(name_tok);
    char *parent_name = "Object";

    if (match(parser, TK_COLON))
    {
        token_t parent_tok = consume(parser, TK_ID, "Expect parent class name after ':'.");
        parent_name = token_value(parent_tok);
    }

    consume(parser, TK_LBRACE, "Expect '{' before class body.");

    emit_classMap(parser, class_name);
    add_variable(parser->comp, class_name);

    emit_boundMethodCall(parser, "Object", "extends", 2);
    load_variable(parser->comp, parent_name);
    load_variable(parser->comp, class_name);
    emit_8u(parser->comp, OP_CALL_FUNCTION, "extends", 2);
    emit(parser->comp, OP_POP);

    emit_boundMethodCall(parser, class_name, "setName", 1);
    int name_index = store_const(parser->comp, NEW_OBJ(new_pistring(class_name)));
    emit_16u(parser->comp, OP_LOAD_CONST, class_name, name_index);
    emit_8u(parser->comp, OP_CALL_FUNCTION, "setName", 1);
    emit(parser->comp, OP_POP);

    emit_boundMethodCall(parser, "Object", "lock", 2);
    load_variable(parser->comp, class_name);
    int lock_index = store_const(parser->comp, NEW_BOOL(true));
    emit_16u(parser->comp, OP_LOAD_CONST, "true", lock_index);
    emit_8u(parser->comp, OP_CALL_FUNCTION, "lock", 2);
    emit(parser->comp, OP_POP);

    emit_boundMethodCall(parser, "Object", "bracketAccess", 2);
    load_variable(parser->comp, class_name);
    int bracket_index = store_const(parser->comp, NEW_BOOL(false));
    emit_16u(parser->comp, OP_LOAD_CONST, "false", bracket_index);
    emit_8u(parser->comp, OP_CALL_FUNCTION, "bracketAccess", 2);
    emit(parser->comp, OP_POP);
}
/**
 * func_decl -> "fun" IDENT "(" param_list ")" block
 * Parses a function declaration, which is a statement that declares a function.
 * @param parser The parser structure containing the tokens to be parsed.
 */
static void func_decl(parser_t *parser)
{
    token_t token = previous(parser);

    if (match(parser, TK_ID))
    {
        token_t id_token = previous(parser); // Capture token for position
        char *name = token_value(id_token);

        if (is_localScope(parser->comp))
            add_local(parser->comp, name);

        consume(parser, TK_LPAREN, "Expect '(' after function name.");
        list_t *params = param_list(parser);
        int size = list_size(params);
        consume(parser, TK_RPAREN, "Expect ')' before function body.");
        consume(parser, TK_LBRACE, "Expect '{' before function body.");
        token = previous(parser);

        push_function(parser->comp, name);
        parser->comp->current->param_names = params;

        // Add parameters as locals
        for (int i = 0; i < size; i++)
            add_local(parser->comp, string_get(params, i));
        add_local(parser->comp, "args");
        add_local(parser->comp, "kw_args");

        bool hit_finalReturn = false;

        while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
        {
            if (hit_finalReturn)
            {
                token_t _token = peek(parser);
                p_errorf(_token.line, _token.column,
                         "Unreachable code after final return statement");
            }

            if (check(parser, TK_RETURN))
            {
                declaration(parser);
                hit_finalReturn = true;
                continue;
            }

            declaration(parser);
        }

        // Implicit return if no return seen
        if (!parser->is_return)
        {
            //  Important: Mark where the implicit return comes from
            token_t rbrace = peek(parser);
            set_pos(parser, rbrace);
            emit(parser->comp, OP_PUSH_NIL);
            emit(parser->comp, OP_RETURN);
        }

        parser->is_return = false;

        consume(parser, TK_RBRACE, "Expect '}' after function body.");

        pop_function(parser->comp, size);

        if (!is_localScope(parser->comp))
        {
            // Mark function definition location before storing it
            set_pos(parser, id_token);
            emit_8u(parser->comp, OP_STORE_GLOBAL, name, store_name(parser->comp, name));
        }
    }
    else
        p_error("Expect function name", token.line, token.column);

    consume_ifExist(parser, 1, TK_SEMICOLON);
}

/**
 * Outputs a debug operation for the parser.
 * Emits the OP_DEBUG operation and consumes an optional semicolon.
 * @param parser The parser object used for parsing.
 */
static void debug(parser_t *parser)
{
    emit(parser->comp, OP_DEBUG);             // Emit the debug operation
    consume_ifExist(parser, 1, TK_SEMICOLON); // Consume a semicolon if it exists
}

static char *import_joinParts(token_t *parts, int count)
{
    int total = 0;
    for (int i = 0; i < count; i++)
        total += parts[i].length;

    total += (count - 1); // dots

    char *path = malloc((size_t)total + 1);
    int offset = 0;

    for (int i = 0; i < count; i++)
    {
        memcpy(path + offset, parts[i].start, (size_t)parts[i].length);
        offset += parts[i].length;
        if (i < count - 1)
            path[offset++] = '.';
    }

    path[offset] = '\0';
    return path;
}

/**
 * Emits the OP_LOAD_CONST operation with the module path and stores the module name in the constants table.
 * Emits the OP_IMPORT operation to load the module.
 * @param parser The parser object used for parsing.
 * @param parts The array of tokens representing the module path.
 * @param count The number of tokens in the module path.
 */

static void emit_importModule(parser_t *parser, token_t *parts, int count)
{
    char *module_path = import_joinParts(parts, count);
    int module_index = store_const(parser->comp, NEW_OBJ(new_pistring(module_path)));
    emit_16u(parser->comp, OP_LOAD_CONST, module_path, module_index);
    emit(parser->comp, OP_IMPORT);
}
/**
 * Emits the OP_LOAD_CONST operation with the export name and the alias name.
 * Emits the OP_GET_EXPORT operation to retrieve the export.
 * Adds a new variable to the current scope with the alias name.
 * @param parser The parser object used for parsing.
 * @param exporttok The token containing the name of the export to be retrieved.
 * @param aliastok The token containing the name of the alias for the export.
 */

static void emit_importBinding(parser_t *parser, token_t export_tok, token_t alias_tok)
{
    // Get the export name and alias name from the tokens
    char *alias_name = token_value(alias_tok);
    int export_index = store_const(parser->comp, new_value(export_tok));

    // Emit the export name and alias name as constants
    emit_16u(parser->comp, OP_LOAD_CONST, alias_name, export_index);
    emit(parser->comp, OP_GET_EXPORT);

    // Add the alias to the current scope
    store_variable(parser->comp, alias_name);
    free(alias_name);
}

static void emit_importAlias(parser_t *parser, token_t alias_tok)
{
    char *alias_name = token_value(alias_tok);
    store_variable(parser->comp, alias_name);
    free(alias_name);
}

/**
 * import_stmt supports:
 * import math
 * import math.*
 * import math.{sin, cos}
 * import math.{sin:s, cos}
 * import math.sin:s
 * import path.to.mod.elem:alias
 *
 * Notes:
 * - Plain import binds the module to its last segment.
 * - `.*` imports all exports into current globals.
 * - Braced/single selectors import selected exports with optional aliases.
 */
static void import_stmt(parser_t *parser)
{
    token_t tok = previous(parser); // 'import'
    set_pos(parser, tok);

    token_t parts[256];
    int count = 0;
    bool import_all = false;
    bool import_braced = false;

    parts[count++] = consume(parser, TK_ID, "Expect module name after 'import'.");

    while (match(parser, TK_DOT))
    {
        if (check(parser, TK_MULT))
        {
            next(parser);
            import_all = true;
            break;
        }

        if (check(parser, TK_LBRACE))
        {
            import_braced = true;
            break;
        }

        if (count >= 256)
            p_error("Import path is too long.", peek(parser).line, peek(parser).column);
        parts[count++] = consume(parser, TK_ID, "Expect identifier after '.'.");
    }

    if (import_all)
    {
        emit_importModule(parser, parts, count);
        emit(parser->comp, OP_IMPORT_ALL);
        consume_ifExist(parser, 1, TK_SEMICOLON);
        return;
    }

    if (import_braced)
    {
        emit_importModule(parser, parts, count); // leaves module on stack

        consume(parser, TK_LBRACE, "Expect '{' after module path.");
        do
        {
            token_t export_tok = consume(parser, TK_ID, "Expect export name inside import list.");
            token_t alias_tok = export_tok;
            if (match(parser, TK_COLON))
                alias_tok = consume(parser, TK_ID, "Expect alias name after ':'.");

            emit(parser->comp, OP_DUP_TOP);
            emit_importBinding(parser, export_tok, alias_tok);

        } while (match(parser, TK_COMMA));

        consume(parser, TK_RBRACE, "Expect '}' after import list.");
        emit(parser->comp, OP_POP); // discard module left on stack
        consume_ifExist(parser, 1, TK_SEMICOLON);
        return;
    }

    if (match(parser, TK_COLON))
    {
        token_t alias_tok = consume(parser, TK_ID, "Expect alias name after ':'.");

        // import module:alias
        if (count == 1)
        {
            emit_importModule(parser, parts, count);
            emit_importAlias(parser, alias_tok);
            consume_ifExist(parser, 1, TK_SEMICOLON);
            return;
        }

        // import path.to.mod.elem:alias
        token_t export_tok = parts[count - 1];

        emit_importModule(parser, parts, count - 1);
        emit_importBinding(parser, export_tok, alias_tok);
        consume_ifExist(parser, 1, TK_SEMICOLON);
        return;
    }

    // Plain module import: bind to export if same-name function exists, else module.
    emit_importModule(parser, parts, count);
    char *binding_name = token_value(parts[count - 1]);
    int name_index = store_const(parser->comp, new_value(parts[count - 1]));
    emit_16u(parser->comp, OP_LOAD_CONST, binding_name, name_index);
    emit(parser->comp, OP_IMPORT_DEFAULT);
    store_variable(parser->comp, binding_name);
    free(binding_name);
    consume_ifExist(parser, 1, TK_SEMICOLON);
}

/**
 * statement -> block | if_stmt | while_stmt | for_stmt | break_stmt | continue_stmt | return_stmt | expr_state
 * Parses a statement, which is a single expression or a block of expressions.
 * @param parser The parser object used for parsing.
 */
static void statement(parser_t *parser)
{
    if (is_destructure_assign(parser))
        destructure_assignStatement(parser);
    else if (match(parser, TK_LBRACE))
    {
        // Look ahead to check if it's an object literal (key: value format)
        int current = parser->current; // Save current position

        if (match_n(parser, 5, TK_STR, TK_ID, TK_NUM, TK_FALSE, TK_TRUE) && match(parser, TK_COLON))
        {
            // If we find key-value pattern, reset position and parse as object
            parser->current = current - 1;
            primary(parser);
        }
        else
        {
            // Otherwise, parse as a block
            parser->current = current; // Restore position
            block(parser);
        }
    }
    else if (match(parser, TK_IF))
        if_stmt(parser);
    else if (match(parser, TK_WHILE))
        while_stmt(parser);
    else if (match(parser, TK_FOR))
        for_stmt(parser);
    else if (match(parser, TK_BREAK))
        break_stmt(parser);
    else if (match(parser, TK_CONTINUE))
        continue_stmt(parser);
    else if (match(parser, TK_RETURN))
        return_stmt(parser);
    else if (match(parser, TK_DEBUG))
        debug(parser);
    else if (match(parser, TK_IMPORT))
        import_stmt(parser);
    else
        expr_state(parser);

    // parser->is_return = false;
}

/**
 * Parses a destructuring assignment statement.
 * Destructuring assignment statements are in the form of [x, y, z] = [a, b, c],
 * where x, y, and z are assigned the values of a, b, and c respectively.
 * @param parser The parser object used for parsing.
 */
static void destructure_assignStatement(parser_t *parser)
{
    // Parse the destructuring assignment statement
    // List to store the identifiers on the left-hand side of the assignment
    list_t *targets = list_create(sizeof(char *));
    // Consume the left brace token
    consume(parser, TK_LBRACKET, "Expect '[' to start destructuring assignment.");
    // Parse the identifiers on the left-hand side of the assignment
    do
    {
        // Consume the identifier token and store its value in the list
        token_t name_tok = consume(parser, TK_ID, "Expect identifier in destructuring assignment.");
        char *name = token_value(name_tok);
        list_add(targets, &name);
    } while (match(parser, TK_COMMA));

    // Consume the right brace token
    consume(parser, TK_RBRACKET, "Expect ']' after destructuring targets.");
    // Consume the assignment operator token
    consume(parser, TK_ASSIGN, "Expect '=' after destructuring targets.");
    // Parse the right-hand side of the assignment
    expr(parser);

    // Get the size of the list
    int size = list_size(targets);
    // Iterate over the list and create bytecode to assign the values
    for (int i = 0; i < size; i++)
    {
        char *name = *(char **)list_getAt(targets, i);
        int index = store_const(parser->comp, NEW_NUM(i));

        // Duplicate the top of the stack (the value to be assigned)
        emit(parser->comp, OP_DUP_TOP);
        // Load the constant value at the specified index
        emit_16u(parser->comp, OP_LOAD_CONST, name, index);
        // Get the item at the specified index from the value
        emit(parser->comp, OP_GET_ITEM);
        // Store the value in the variable
        store_variable(parser->comp, name);
    }

    // Pop the value from the stack
    emit(parser->comp, OP_POP);

    // Check if a delimiter is needed
    if (need_delimiter(parser))
        p_error("Expected delemiter between statements.", peek(parser).line, peek(parser).column);
}

/**
 * Executes a block of code by creating a new scope.
 * Parses and processes declarations until a closing brace '}' or end of input is encountered.
 * Pops the scope after processing the block, ensuring proper scope management.
 * Consumes the closing brace token to validate block syntax.
 * @returns nothing
 */
static void block(parser_t *parser)
{
    // Create a new scope for the block
    push_scope(parser->comp);

    // Parse and process declarations until the closing brace or end of input is encountered
    while (!check(parser, TK_RBRACE) && !is_atEnd(parser) && !parser->is_return)
        declaration(parser);

    if (parser->is_return && !check(parser, TK_RBRACE))
        p_error("Unreachable code after return statement.", peek(parser).line, peek(parser).column);

    parser->is_return = false;

    // Pop the scope after processing the block
    pop_scope(parser->comp);

    // Consume the closing brace token to validate block syntax
    consume(parser, TK_RBRACE, "Expect '}' after block.");
}

/**
 * print -> primary
 * Parses a print statement, which is a statement that prints its argument to the console.
 * Emits the OP_PRINT bytecode to print the result of the expression.
 * Consumes the semicolon token to validate the statement syntax.
 * @returns nothing
 */
static void print(parser_t *parser)
{
    primary(parser);                          // Parse the expression to be printed
    emit(parser->comp, OP_PRINT);             // Emit bytecode to print the result of the expression
    consume_ifExist(parser, 1, TK_SEMICOLON); // Consume the semicolon token to validate the statement syntax
}

/**
 * condition -> "(" expr ")" condition?
 *
 * Parses a condition expression, which is an expression enclosed in parentheses.
 * The condition expression is parsed by calling the cond_expr() function.
 * @param parser The parser object used for parsing.
 */
static void condition(parser_t *parser)
{
    bool has_parens = match(parser, TK_LPAREN); // Match and consume '(' if present

    cond_expr(parser); // Your existing function to parse the condition expression

    if (has_parens)
        consume(parser, TK_RPAREN, "Expect ')' after condition.");
}
/**
 * if_stmt -> "if" "(" expr ")" block ("elif" "(" expr ")" block)* ("else" block)?
 * Parses an if statement with optional elif and else clauses.
 */
static void if_stmt(parser_t *parser)
{
    token_t start = peek(parser); // capture for accurate position
    condition(parser);

    set_pos(parser, start);
    int then_jump = emit_16u(parser->comp, OP_JUMP_IF_FALSE, "", 0);

    if (match(parser, TK_LBRACE))
        block(parser);
    else
    {
        statement(parser);
        parser->is_return = false;
    }

    int end_jumps[256];
    int jump_count = 0;

    if (check(parser, TK_ELIF) || check(parser, TK_ELSE))
    {
        set_pos(parser, peek(parser));
        end_jumps[jump_count++] = emit_16u(parser->comp, OP_JUMP, "", 0);
    }

    patch_jump(parser->comp, then_jump);

    while (match(parser, TK_ELIF))
    {
        token_t elif_tok = previous(parser);
        condition(parser);

        set_pos(parser, elif_tok);
        then_jump = emit_16u(parser->comp, OP_JUMP_IF_FALSE, "", 0);

        if (match(parser, TK_LBRACE))
            block(parser);
        else
        {
            statement(parser);
            parser->is_return = false;
        }

        if (check(parser, TK_ELIF) || check(parser, TK_ELSE))
        {
            set_pos(parser, peek(parser));
            end_jumps[jump_count++] = emit_16u(parser->comp, OP_JUMP, "", 0);
        }

        patch_jump(parser->comp, then_jump);
    }

    if (match(parser, TK_ELSE))
    {
        token_t else_tok = previous(parser);
        set_pos(parser, else_tok);

        patch_jump(parser->comp, then_jump);

        if (match(parser, TK_LBRACE))
            block(parser);
        else
        {
            statement(parser);
            parser->is_return = false;
        }
    }
    else
        patch_jump(parser->comp, then_jump);

    for (int i = 0; i < jump_count; i++)
        patch_jump(parser->comp, end_jumps[i]);
}

/**
 * while_stmt -> "while" "(" expr ")" block
 * Parses a while loop, which repeatedly executes a block as long as a condition is true.
 * @param parser The parser object used for parsing.
 */
static void while_stmt(parser_t *parser)
{
    // Record the address to jump back to for looping
    int jump = code_size(parser->comp);

    // Capture the starting position of the condition for error reporting
    token_t cond_start = peek(parser);

    // Parse the loop condition
    condition(parser);

    // Set the parser position to the start of the condition
    set_pos(parser, cond_start);

    // Emit a conditional jump instruction to exit the loop if the condition is false
    int address = emit_16u(parser->comp, OP_JUMP_IF_FALSE, "", 0);

    // Push a new loop context onto the stack
    push_loop(parser->comp, jump, false);

    // Check if the loop body is enclosed in braces and parse accordingly
    if (match(parser, TK_LBRACE))
        block(parser);
    else
    {
        statement(parser);
        parser->is_return = false;
    }

    // Pop the loop context and patch the jump address to loop back
    pop_loop(parser->comp, jump);
    patch_jump(parser->comp, address);
}

/**
 * for_stmt -> "for" "(" IDENT "in" expr ")" block
 * Parses a for-in loop, which iterates over the elements of an iterable.
 */
static void for_stmt(parser_t *parser)
{
    bool has_parens = match(parser, TK_LPAREN);

    token_t init = consume(parser, TK_ID, "Invalid for-loop left-hand side. Expect identifier.");

    consume(parser, TK_IN, "Expect 'in' keyword after loop variable.");

    token_t cond_tok = peek(parser);
    cond_expr(parser);

    if (has_parens)
        consume(parser, TK_RPAREN, "Expect ')' after iterable expression.");

    set_pos(parser, cond_tok); // associate with iterable expression
    emit(parser->comp, OP_PUSH_ITER);

    set_pos(parser, init); // mark the loop start
    int address = emit_16u(parser->comp, OP_LOOP, "", 0);

    push_scope(parser->comp);

    add_variable(parser->comp, token_value(init));
    push_loop(parser->comp, address - 2, true);

    if (match(parser, TK_LBRACE))
    {
        while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
            declaration(parser);

        consume(parser, TK_RBRACE, "Expect '}' after block.");
    }
    else
    {
        statement(parser);
        parser->is_return = false;
    }

    pop_scope(parser->comp);
    pop_loop(parser->comp, address - 2);
    patch_jump(parser->comp, address);
}

/**
 * break_stmt -> "break"
 * Parses a break statement, which is used to prematurely exit a loop.
 * Emits the OP_POP_ITER bytecode to remove the loop iterator from the stack.
 * If the break statement is inside a for loop, a jump is emitted to the end of
 * the loop. Otherwise, the code will exit the loop and continue executing the
 * code after the loop.
 * @param parser The parser object used for parsing.
 */
static void break_stmt(parser_t *parser)
{
    token_t tok = previous(parser); // 'break' token
    set_pos(parser, tok);

    if (!in_loop(parser->comp))
        p_errorf(tok.line, tok.column, "'break' used outside of a loop");

    if (is_forLoop(parser->comp))
        emit(parser->comp, OP_POP_ITER);

    emit_pop(parser->comp, loop_depth(parser->comp));
    push_break(parser->comp, emit_jump(parser->comp, 0));

    // Mark this point as a return-like exit to check for unreachable code
    parser->is_return = true;

    if (need_delimiter(parser))
        p_error("Expected delimiter or newline after 'break'.", tok.line, tok.column);
}

/**
 * continue_stmt -> "continue"
 * Parses a continue statement, which is used to skip the current iteration of a loop.
 * It emits the necessary bytecode to jump to the start of the loop.
 * @param parser The parser object used for parsing.
 */
static void continue_stmt(parser_t *parser)
{
    token_t tok = previous(parser); // 'continue' token
    set_pos(parser, tok);

    if (!in_loop(parser->comp))
        p_errorf(tok.line, tok.column, "'continue' used outside of a loop");

    int address = get_continue(parser->comp);
    emit_pop(parser->comp, loop_depth(parser->comp));
    emit_jump(parser->comp, address - code_size(parser->comp));

    parser->is_return = true;

    if (need_delimiter(parser))
        p_error("Expected delemiter or newline after 'continue'.", tok.line, tok.column);
}

/**
 * return_stmt -> "return [expr]?"
 * Parses a return statement, optionally with a return value.
 */
static void return_stmt(parser_t *parser)
{
    token_t tok = previous(parser); // 'return' token
    set_pos(parser, tok);

    if (is_constructor(parser->comp))
    {
        if (!check(parser, TK_SEMICOLON) && !is_lineBreak(parser))
            p_error("Constructors cannot return a value.", peek(parser).line, peek(parser).column);

        emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
    }
    else
    {
        // Check if return is followed by a newline or semicolon => nil
        if (match(parser, TK_SEMICOLON) || is_lineBreak(parser))
        {
            int index = store_const(parser->comp, NEW_NIL());
            emit_16u(parser->comp, OP_LOAD_CONST, "nil", index);
        }
        else
            expr(parser); // return with value
    }

    emit(parser->comp, OP_RETURN);
    parser->is_return = true;

    if (need_delimiter(parser))
        p_error("Expected delemiter or newline after return.", tok.line, tok.column);
}

/**
 * expr_state -> expr
 * Parses an expression statement.
 * An expression statement is an expression followed by a semicolon.
 * The expression is evaluated and the result is discarded.
 * @returns nothing
 */
static void expr_state(parser_t *parser)
{

    token_t token = peek(parser);
    int start_line = token.line;
    bool prev_lookUp, is_assign = false;
    int current = parser->current;

    // Check if the expression is enclosed in parentheses
    if (token.type == TK_LPAREN)
    {
        prev_lookUp = look_up(parser->comp, true); // Set the look_up flag to true to indicate that the expression is enclosed in parentheses
        primary(parser);                           // Parse the primary expression
        look_up(parser->comp, prev_lookUp);        // Reset the look_up flag to false after parsing the expression
        parser->current = current;                 // Reset the current position to its original value
    }

    current = parser->current;

    prev_lookUp = look_up(parser->comp, true);

    // Check if the expression is an assignment expression
    cond_expr(parser);
    token = peek(parser);
    if (token.line == start_line && token.type >= TK_ASSIGN && token.type <= TK_MOD_ASSIGN)
        is_assign = true;
    look_up(parser->comp, prev_lookUp);

    parser->current = current;

    expr(parser); // Parse the expression

    // Check if the expression is an assignment expression
    // If it is, do not emit the POP bytecode
    // The assignment expression is handled separately
    if (!is_assign)
    {
        if (!parser->comp->is_repl)
            emit(parser->comp, OP_POP); // Emit POP only if not in REPL mode
    }

    // Check for statement separation
    if (need_delimiter(parser))
        p_error("Expected delemiter between statements.", peek(parser).line, peek(parser).column);
}

/**
 * expr -> assignment
 * Parses an expression, which is a statement that assigns a value to a variable.
 * The assignment expression can be a simple assignment or a compound assignment
 * like +=, -=, \*=, /=, %=, |=, ^=, or &=.
 * @returns nothing
 */
static void expr(parser_t *parser)
{
    assignment(parser, false); // Parse the assignment expression
}

/**
 * Initializes a new assign_t structure with the given fields.
 * @param left the first index of left hand side of the assignment
 * @param right the first index of right hand side of the assignment
 * @param op the operator used in the assignment
 * @return a pointer to the newly allocated assign_t structure
 */
static assign_t *init_assign(int left, int right, tk_type op)
{
    // Allocate memory for a new assign_t structure
    assign_t *assign = malloc(sizeof(assign_t));

    // Initialize fields
    assign->left = left;
    assign->right = right;
    assign->op = op;

    return assign;
}

/**
 * assignment -> condition
 * Parses an assignment expression, which is a statement that assigns a value
 * to a variable. It supports compound assignments like +=, -=, \*=, /=, %=,
 * |=, ^=, and &=.
 * @returns nothing
 */
static void assignment(parser_t *parser, bool emit_load)
{
    tk_type op;
    stack_t *assigns = stack_create(sizeof(assign_t));
    int left = parser->current, right;
    bool prev_lookUp = look_up(parser->comp, true);

    // First pass: collect assignments in the stack without emitting bytecode
    // This is done to handle the case where there are multiple assignments in
    // a single expression, e.g. "a = b = c = d = 0".
    cond_expr(parser);
    while (match_n(parser, 9, TK_ASSIGN, TK_PLUS_ASSIGN, TK_MINUS_ASSIGN, TK_DIV_ASSIGN, TK_MULT_ASSIGN,
                   TK_MOD_ASSIGN, TK_BITOR_ASSIGN, TK_XOR_ASSIGN, TK_BITAND_ASSIGN))
    {

        op = previous(parser).type;

        right = parser->current;

        // Push the assignment information to the stack
        push(assigns, init_assign(left, right, op));

        // Parse the right-hand side of the assignment
        cond_expr(parser);
        left = right;
    }

    look_up(parser->comp, prev_lookUp);

    // If there was no assignment operator, re-evaluate as a non-assignment
    if (is_empty(assigns))
    {
        parser->current = left;
        cond_expr(parser); // Re-evaluate as a non-assignment expression
    }
    else
    {
        int current = parser->current;
        assign_t *assign;

        // Second pass: pop each assignment and generate bytecode
        while (!is_empty(assigns))
        {

            assign = pop(assigns);

            op = assign->op;
            left = assign->left;
            right = assign->right;

            token_t lhs = parser->tokens[left];

            if (parser->tokens[left].type != TK_ID)
                p_error("Invalid assignment target", parser->tokens[left].line, parser->tokens[left].column);

            // Sync the runtime error position to LHS token
            set_pos(parser, lhs);

            if (op != TK_ASSIGN)
            {
                // Load LHS for compound assignments
                parser->current = left;
                cond_expr(parser);
            }

            // Evaluate RHS
            parser->current = right;
            char *prev_fun = parser->fun_name;
            char *prev_obj = parser->object_name;

            if (op == TK_ASSIGN && is_functionLiteral(parser, right))
                parser->fun_name = token_value(lhs);
            if (op == TK_ASSIGN && is_objectLiteral(parser, right))
                parser->object_name = token_value(lhs);

            cond_expr(parser);
            parser->fun_name = prev_fun;
            parser->object_name = prev_obj;

            // Emit the bytecode for compound operation (e.g., `+=`)
            if (op != TK_ASSIGN)
            {
                // Emit the bytecode for the compound assignment
                switch (op)
                {
                case TK_PLUS_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[0], 0); // OP_BINARY_ADD
                    break;
                case TK_MINUS_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[1], 1); // OP_BINARY_SUB
                    break;
                case TK_MULT_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[2], 2); // OP_BINARY_MUL
                    break;
                case TK_DIV_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[3], 3); // OP_BINARY_DIV
                    break;
                case TK_MOD_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[4], 4); // OP_BINARY_MOD
                    break;
                case TK_BITOR_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[5], 5); // OP_BINARY_BITOR
                    break;
                case TK_XOR_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[6], 6); // OP_BINARY_XOR
                    break;
                case TK_BITAND_ASSIGN:
                    emit_8u(parser->comp, OP_BINARY, bin_ops[7], 7); // OP_BINARY_BITAND
                    break;
                default:
                    break;
                }
            }

            // Store result to LHS
            parser->current = left;
            parser->is_store = true;
            cond_expr(parser);
        }

        if (emit_load)
        {
            parser->current = left;
            cond_expr(parser);
        }

        // Restore the original parsing position
        parser->current = current;
    }
}

/**
 * cond_expr -> or_expr ("?" expr ":" expr)?
 *
 * Parse a conditional expression. If the condition is a ternary expression,
 * parse the expression after the '?' and the expression after the ':'. If the
 * condition is not a ternary expression, just parse the expression.
 *
 * @returns nothing
 */
static void cond_expr(parser_t *parser)
{

    or_expr(parser);

    if (match(parser, TK_QUESTION))
    {
        /*
         * Emit a jump if the condition is false. The jump offset is initially
         * set to 0, and the address of the jump instruction is stored in
         * then_jump. The jump offset is patched later when the actual address
         * of the target instruction is known.
         */
        int then_jump = emit_16u(parser->comp, OP_JUMP_IF_FALSE, "", 0);

        // Sync current token for better runtime error info
        set_pos(parser, peek(parser));

        /*
         * Parse the expression after the '?'. This is the expression that will
         * be executed if the condition is true.
         */
        cond_expr(parser);

        /*
         * Parse the expression after the ':'. This is the expression that will
         * be executed if the condition is false.
         */
        token_t token = consume(parser, TK_COLON, "Expect ':' after '?'");
        int else_jump = emit_16u(parser->comp, OP_JUMP, "", 0);

        /*
         * Patch the jump offset of the jump instruction stored in then_jump
         * with the current instruction address. This will cause the jump
         * instruction to jump to the instruction after the '?' expression.
         */
        patch_jump(parser->comp, then_jump);

        /*
         * Parse the expression after the ':'. This is the expression that will
         * be executed if the condition is false.
         */
        cond_expr(parser);

        /*
         * Patch the jump offset of the jump instruction stored in else_jump
         * with the current instruction address. This will cause the jump
         * instruction to jump to the instruction after the ':' expression.
         */
        patch_jump(parser->comp, else_jump);
    }
}

/**
 * or_expr -> and_expr ("or" and_expr)*
 * Parses a logical OR expression, which is an expression that checks if either
 * of two values are true. The syntax for a logical OR expression is [value1 or
 * value2] or [value1 or value2 or value3].
 * @returns nothing
 */
static void or_expr(parser_t *parser)
{
    and_expr(parser);
    // Parse the "or" expression
    while (match(parser, TK_OR))
    {
        token_t op_token = previous(parser);
        and_expr(parser);
        set_pos(parser, op_token);
        // Emit bytecode for the logical OR operator
        emit_8u(parser->comp, OP_BINARY, bin_ops[6], 6);
    }
}

/**
 * and_expr -> in_expr ("and" in_expr)*
 * Parses a logical AND expression, which is an expression that checks if two
 * values are true. The syntax for a logical AND expression is [value1 and value2]
 * or [value1 and value2 and value3].
 * @returns nothing
 */
static void and_expr(parser_t *parser)
{
    in_expr(parser);
    while (match(parser, TK_AND))
    {
        token_t op_token = previous(parser);
        in_expr(parser);
        set_pos(parser, op_token);
        emit_8u(parser->comp, OP_BINARY, bin_ops[5], 5); // Emit bytecode for the "and" operator
    }
}

/**
 * in_expr -> range_expr ( "in" range_expr )*
 * Parses a membership expression, which is an expression that checks if a
 * value is in a list or tuple. The syntax for a membership expression is
 * [value in list] or [value in list if condition].
 * @returns nothing
 */
static void in_expr(parser_t *parser)
{
    range_expr(parser);
    while (match(parser, TK_IN))
    {
        token_t op_token = previous(parser);
        range_expr(parser);
        set_pos(parser, op_token);
        emit_8u(parser->comp, OP_COMPARE, comp_ops[6], 6); // Emit bytecode for the "in" operator
    }
}

/**
 * range_expr -> bitOr_expr ( ".." bitOr_expr (":" expr?)? )?
 * Parses a range expression, which is a form of slicing a list or tuple.
 * The syntax for a range expression is [start..stop] or [start..stop:step],
 * where start, stop, and step are optional and default to 0, the size of the
 * list, and 1, respectively.
 * @returns nothing
 */
static void range_expr(parser_t *parser)
{
    bitOr_expr(parser);
    if (match(parser, TK_DBDOTS))
    {
        token_t op_token = previous(parser);
        bitOr_expr(parser);
        if (match(parser, TK_COLON))
            expr(parser); // parse the step
        else
            emit(parser->comp, OP_PUSH_NIL);
        set_pos(parser, op_token);
        // generate the bytecode for the range expression here
        emit(parser->comp, OP_PUSH_RANGE);
    }
}

/**
 * bitOr_expr -> xor_expr ( "|" xor_expr )*
 * Parses a bitwise OR expression.
 * @returns nothing
 */
static void bitOr_expr(parser_t *parser)
{
    xor_expr(parser);
    while (match(parser, TK_BITOR))
    {
        token_t op_token = previous(parser);
        xor_expr(parser);
        set_pos(parser, op_token);
        // generate the bytecode for the binary expression here
        emit_8u(parser->comp, OP_BINARY, bin_ops[9], 9);
    }
}

/**
 * xor_expr -> bitAnd_expr ( "^" bitAnd_expr )*
 * Parses a bitwise XOR expression.
 * @returns nothing
 */
static void xor_expr(parser_t *parser)
{
    bitAnd_expr(parser);
    while (match(parser, TK_XOR))
    {
        token_t op_token = previous(parser);
        bitAnd_expr(parser);
        set_pos(parser, op_token);
        // generate the bytecode for the binary expression here
        emit_8u(parser->comp, OP_BINARY, bin_ops[10], 10);
    }
}

/**
 * bitAnd_expr -> shift_expr ( "&" shift_expr )*
 * Parses a bitwise AND expression.
 * @returns nothing
 */
static void bitAnd_expr(parser_t *parser)
{
    shift_expr(parser);
    while (match(parser, TK_BITAND))
    {
        token_t op_token = previous(parser);
        shift_expr(parser);
        set_pos(parser, op_token);
        // generate the bytecode for the binary expression here
        emit_8u(parser->comp, OP_BINARY, bin_ops[8], 8);
    }
}

/**
 * shift_expr -> equality_expr (("<<" | ">>" | ">>>") equality_expr)*
 * Parses a shift expression, which allows shifting bits to the left or right.
 * The supported operators are <<, >>, and >>> for left, right, and unsigned right shifts respectively.
 * Emits the corresponding bytecode for the parsed expression.
 */
static void shift_expr(parser_t *parser)
{
    equality_expr(parser); // Parse the initial equality expression

    // Loop to handle multiple shift operations
    while (match_n(parser, 3, TK_LSHIFT, TK_RSHIFT, TK_URSHIFT))
    {
        tk_type op = previous(parser).type; // Get the shift operator
        token_t op_token = previous(parser);
        equality_expr(parser); // Parse the right-hand side expression
        set_pos(parser, op_token);

        // Emit the bytecode for the corresponding shift operation
        switch (op)
        {
        case TK_LSHIFT:
            emit_8u(parser->comp, OP_BINARY, bin_ops[11], 11); // Emit bytecode for <<
            break;
        case TK_RSHIFT:
            emit_8u(parser->comp, OP_BINARY, bin_ops[12], 12); // Emit bytecode for >>
            break;
        case TK_URSHIFT:
            emit_8u(parser->comp, OP_BINARY, bin_ops[13], 13); // Emit bytecode for >>>
            break;
        default:
            break;
        }
    }
}

/**
 * equality_expr -> compare_expr (("!=" | "==" | "is") compare_expr)*
 * Parses an equality expression, which is an expression that compares two
 * values for equality or inequality. The equality operators are != and ==.
 * Emits the corresponding bytecode for the parsed expression.
 */

static void equality_expr(parser_t *parser)
{
    compare_expr(parser);
    while (match_n(parser, 3, TK_NOT_EQUAL, TK_EQUAL, TK_IS))
    {
        tk_type op = previous(parser).type;
        token_t op_token = previous(parser);
        compare_expr(parser);
        set_pos(parser, op_token);

        if (op == TK_NOT_EQUAL)
            // !=
            emit_8u(parser->comp, OP_COMPARE, comp_ops[3], 3);

        else if (op == TK_EQUAL)
            // ==
            emit_8u(parser->comp, OP_COMPARE, comp_ops[2], 2);

        else if (op == TK_IS)
            // is
            emit_8u(parser->comp, OP_BINARY, bin_ops[15], 15);
    }
}

/**
 * compare_expr -> add_expr ((">" | "<" | ">=" | "<=") add_expr)*
 * Parses a comparison expression, which is an expression that compares two
 * values. The comparison operators are >, <, >=, and <=. Emits the corresponding
 * bytecode for the parsed expression.
 * @returns nothing
 */
static void compare_expr(parser_t *parser)
{
    // Parse the first expression (e.g., 'a' in 'a < b < c')
    add_expr(parser);

    // Store the current token position so we can reparse intermediate values
    int last_value_pos = -1;
    int comparison_count = 0;

    while (match_n(parser, 6, TK_EQUAL, TK_NOT_EQUAL, TK_GREATER,
                   TK_LESS, TK_GREATER_EQUAL, TK_LESS_EQUAL))
    {
        tk_type op = previous(parser).type;
        token_t op_token = previous(parser);

        if (last_value_pos != -1)
        {
            // Rewind and reparse the previous right-hand side (e.g., `b`)
            parser->current = last_value_pos;
            add_expr(parser); // push `b` again
            next(parser);     // advance past the operator
        }

        // Save position before parsing the next expression (e.g., `c`)
        last_value_pos = parser->current;
        add_expr(parser); // parse right-hand side
        set_pos(parser, op_token);

        // Emit the comparison operator
        int op_index = -1;
        switch (op)
        {
        case TK_EQUAL:
            op_index = 0;
            break;
        case TK_NOT_EQUAL:
            op_index = 1;
            break;
        case TK_GREATER:
            op_index = 2;
            break;
        case TK_LESS:
            op_index = 3;
            break;
        case TK_GREATER_EQUAL:
            op_index = 4;
            break;
        case TK_LESS_EQUAL:
            op_index = 5;
            break;
        default:
            break;
        }
        emit_8u(parser->comp, OP_COMPARE, comp_ops[op_index], op_index);

        // If this is not the first comparison, chain it with an AND
        if (comparison_count > 0)
            emit_8u(parser->comp, OP_BINARY, bin_ops[5], 5); // logical AND

        comparison_count++;
    }
}

/**
 * add_expr -> mult_expr (("+" | "-") mult_expr)*
 * Parses an addition expression, which is an expression that adds or subtracts
 * two values. The syntax for an addition expression is [value1 + value2] or
 * [value1 - value2]. Emits the corresponding bytecode for the parsed
 * expression.
 * @returns nothing
 */
static void add_expr(parser_t *parser)
{
    dot_expr(parser);
    while (match_n(parser, 2, TK_PLUS, TK_MINUS))
    {

        token_t op = previous(parser);
        dot_expr(parser);
        set_pos(parser, op);
        if (op.type == TK_PLUS)
            emit_8u(parser->comp, OP_BINARY, bin_ops[0], 0);
        else
            emit_8u(parser->comp, OP_BINARY, bin_ops[1], 1);
    }
}

/**
 * dot_expr -> mult_expr ( "." mult_expr )*
 * Parses a dot product expression, which is an expression that takes the dot
 * product of two values. The syntax for a dot product expression is
 * [value1 . value2]. Emits the corresponding bytecode for the parsed
 * expression.
 * @returns nothing
 */
static void dot_expr(parser_t *parser)
{
    mult_expr(parser); // Parse the left-hand side of the dot product
    while (match(parser, TK_DOT_PROD))
    {
        token_t op = previous(parser);                     // Save the dot product operator
        mult_expr(parser);                                 // Parse the right-hand side of the dot product
        set_pos(parser, op);                               // Set the position to the dot product operator
        emit_8u(parser->comp, OP_BINARY, bin_ops[14], 14); // Emit the bytecode
    }
}

/**
 * mult_expr -> exp_expr (("*" | "/" | "%") exp_expr)*
 * Parses a multiplication expression, which is an expression that multiplies,
 * divides, or takes the modulus of two values. The syntax for a multiplication
 * expression is [value1 * value2], [value1 / value2], or [value1 % value2].
 * Emits the corresponding bytecode for the parsed expression.
 * @returns nothing
 */
static void mult_expr(parser_t *parser)
{
    exp_expr(parser);
    while (match_n(parser, 3, TK_MULT, TK_DIV, TK_MOD))
    {
        token_t op = previous(parser);
        exp_expr(parser);
        set_pos(parser, op);
        switch (op.type)
        {
        case TK_MULT:
            // Emit bytecode for the * operator
            emit_8u(parser->comp, OP_BINARY, bin_ops[2], 2);
            break;
        case TK_DIV:
            // Emit bytecode for the / operator
            emit_8u(parser->comp, OP_BINARY, bin_ops[3], 3);
            break;
        case TK_MOD:
            // Emit bytecode for the % operator
            emit_8u(parser->comp, OP_BINARY, bin_ops[4], 4);
            break;
        default:
            break;
        }
    }
}

/**
 * exp_expr -> unary_expr ("**" exp_expr)*
 * Parses an exponentiation expression. It consists of a unary expression
 * that can be followed by one or more exponentiation operations.
 * The right-hand side of the "**" operator is recursively parsed as another
 * exponentiation expression.
 */
static void exp_expr(parser_t *parser)
{
    unary_expr(parser);             // Parse the base unary expression
    while (match(parser, TK_POWER)) // Check for the exponentiation operator
    {
        token_t op = previous(parser);
        exp_expr(parser); // Recursively parse the exponent
        set_pos(parser, op);
        emit_8u(parser->comp, OP_BINARY, bin_ops[7], 7); // Emit bytecode for exponentiation
    }
}

/**
 * unary_expr -> ("+" | "-" | "!" | "~" | "#" | "typeof")* ("++" | "--")? member_expr
 * An unary expression is a single value that can be either a primary (e.g.
 * a number, a string) or a single-expression expression (e.g. a variable, a
 * function call). The value of the unary expression is the value of the
 * expression.
 */
static void unary_expr(parser_t *parser)
{
    tk_type op;
    int current;

    if (match_n(parser, 8, TK_PLUS, TK_MINUS, TK_NOT, TK_BITNEG, TK_HASH, TK_INCR, TK_DECR, TK_TYPEOF))
    {
        op = previous(parser).type;
        token_t op_token = previous(parser);

        // Handle negative number literals directly
        if (op == TK_MINUS && peek(parser).type == TK_NUM)
        {
            parser->tokens[parser->current].is_negative = true;
            member_expr(parser);
            return;
        }

        if (op == TK_INCR || op == TK_DECR)
        {
            current = parser->current;
            member_expr(parser);
            set_pos(parser, op_token);

            token_t target = previous(parser);

            if (target.type == TK_NUM || target.type == TK_STR || target.type == TK_TRUE ||
                target.type == TK_FALSE || target.type == TK_NIL)
                p_error("Increment/Decrement operations cannot be applied to calls or literals.",
                        target.line, target.column);

            int type = (op == TK_INCR) ? 5 : 6;
            emit_8u(parser->comp, OP_UNARY, unary_ops[type], type);

            // DUP: one copy to store, one to leave as expression result
            emit(parser->comp, OP_DUP_TOP);

            parser->current = current;
            parser->is_store = true;
            parser->force_store = true;
            member_expr(parser);
            parser->is_store = false;
            parser->force_store = false;
        }
        else
        {
            int type = -1;
            switch (op)
            {
            case TK_PLUS:
                type = 0;
                break;
            case TK_MINUS:
                type = 1;
                break;
            case TK_NOT:
                type = 2;
                break;
            case TK_BITNEG:
                type = 3;
                break;
            case TK_HASH:
                type = 4;
                break;
            case TK_TYPEOF:
                type = 7;
                break;
            default:
                break;
            }
            unary_expr(parser);
            set_pos(parser, op_token);
            if (type != -1)
                emit_8u(parser->comp, OP_UNARY, unary_ops[type], type);
        }
    }
    else
    {
        current = parser->current;
        member_expr(parser);
        token_t operand = previous(parser);

        // Handle post-increment / post-decrement
        if (match_n(parser, 2, TK_INCR, TK_DECR))
        {
            op = previous(parser).type;
            token_t op_token = previous(parser);

            if (op_token.line != operand.line)
            {
                // Not on the same line, so not post-increment, put back the token
                parser->current--;
            }
            else
            {

                if (operand.type == TK_NUM || operand.type == TK_STR || operand.type == TK_TRUE ||
                    operand.type == TK_FALSE || operand.type == TK_NIL)
                    p_error("Increment/Decrement operations cannot be applied to literals.",
                            operand.line, operand.column);

                emit(parser->comp, OP_DUP_TOP);
                set_pos(parser, op_token);

                int type = (op == TK_INCR) ? 5 : 6;
                emit_8u(parser->comp, OP_UNARY, unary_ops[type], type);

                parser->current = current;
                parser->is_store = true;
                member_expr(parser);
                advance(parser); // Skip over the ++ or --

                parser->is_store = false;
            }
        }
    }
}

/**
 * slice_expr -> (expr? ":" expr? (":" expr?)?)?
 * Parses a slice expression, which is a form of indexing a list or tuple.
 * The syntax for a slice expression is [start:stop:step], where start, stop, and
 * step are optional and default to 0, the size of the list, and 1, respectively.
 * @returns nothing
 */
static bool slice_expr(parser_t *parser)
{
    int index;
    bool is_slice = false;
    token_t token = peek(parser);

    if (check(parser, TK_COLON))
    {
        // Missing start → emit INFINITY so get_slice's isinf() check fires
        emit_16u(parser->comp, OP_LOAD_CONST, "inf", 1);
        is_slice = true;
        next(parser); // consume the leading ':'
    }
    else
        cond_expr(parser);

    // FIX: flip operands so match() is only called when is_slice is false
    if (is_slice || match(parser, TK_COLON))
    {
        is_slice = true;
        token = previous(parser);

        if (!check(parser, TK_RBRACKET) && !check(parser, TK_COLON) && !check(parser, TK_COMMA))
            cond_expr(parser);
        else
            emit_16u(parser->comp, OP_LOAD_CONST, "inf", 1); // missing end → INFINITY

        if (match(parser, TK_COLON))
        {
            if (!check(parser, TK_RBRACKET) && !check(parser, TK_COMMA))
                cond_expr(parser);
            else
            {
                index = store_const(parser->comp, NEW_NUM(1));
                emit_16u(parser->comp, OP_LOAD_CONST, "1", index);
            }
        }
        else
        {
            index = store_const(parser->comp, NEW_NUM(1));
            emit_16u(parser->comp, OP_LOAD_CONST, "1", index);
        }

        set_pos(parser, token);
        emit(parser->comp, OP_PUSH_SLICE);
    }
    return is_slice;
}

/**
 * Parses a member expression, which is an expression that accesses a property
 * or method of an object. A member expression can include property access
 * using a dot (.) or bracket ([]) notation, as well as function calls.
 * @returns nothing
 */
static void member_expr(parser_t *parser)
{
    primary(parser); // Parse the primary expression (e.g., variable or literal)

    while (true)
    {
        token_t token = previous(parser);

        if (token.line < peek(parser).line)
            break;

        set_pos(parser, token);
        if (match(parser, TK_DOT))
        {
            token_t token = previous(parser); // For position tracking
            // Handle property access using dot notation
            token_t name = consume(parser, TK_ID, "Expect property name after '.'");

            int index = store_const(parser->comp, new_value(name)); // Store the property name as a constant
            emit_16u(parser->comp, OP_LOAD_CONST, token_value(name), index);

            bool is_chained_access = parser->is_store && parser->force_store &&
                                     has_accessContinuation(parser, name);

            if (!is_chained_access && is_assign(parser))
                emit(parser->comp, OP_SET_MEMBER); // Emit bytecode to set the property value
            else
                emit(parser->comp, OP_GET_MEMBER); // Emit bytecode to get the property value
        }


        // Handle property access using bracket notation and slicing for lists and tensors
        else if (match(parser, TK_LBRACKET))
        {
            token_t token = previous(parser);
            bool is_slice = slice_expr(parser); // still need return value for 1D

            if (check(parser, TK_COMMA))
            {
                /*  Tensor indexing: tensor[i, j:k, m, ...]  */
                int ndim = 1;
                while (match(parser, TK_COMMA))
                {
                    if (check(parser, TK_RBRACKET))
                        break; /* trailing comma */

                    if (ndim >= MAX_TENSOR_DIMS)
                        p_error("Too many tensor dimensions",
                                peek(parser).line, peek(parser).column);

                    slice_expr(parser); /* return value discarded, VM checks at runtime */
                    ndim++;
                }

                // Check for chained access before deciding whether this is an assignment to a tensor element or just an access
                consume(parser, TK_RBRACKET, "Expect ']' after tensor index");
                bool is_chained_access = parser->is_store && parser->force_store &&
                                         has_accessContinuation(parser, previous(parser));
                bool assign = !is_chained_access && is_assign(parser);

                /* VM pops ndim values and checks each at runtime for slice vs index */
                emit_8u(parser->comp, assign ? OP_TENSOR_SET : OP_TENSOR_GET, "", (uint8_t)ndim);
            }
            else
            {
                /*  List / 1-D indexing: list[i] or list[a:b:c]  */
                consume(parser, TK_RBRACKET, "Expect ']' after index");
                bool is_chained_access = parser->is_store && parser->force_store &&
                                         has_accessContinuation(parser, previous(parser));

                // Always emit GET_ITEM or SET_ITEM – the slice object is already on the stack
                bool assign = !is_chained_access && is_assign(parser);
                emit(parser->comp, assign ? OP_SET_ITEM : OP_GET_ITEM);
            }
        }

        // handle function call
        else if (match(parser, TK_LPAREN))
        {
            int args = 0;
            int named = 0;
            bool saw_named = false;
            bool saw_spread = call_hasSpreadArgs(parser);

            if (token.type == TK_SUPER)
            {
                int ctor_index = store_const(parser->comp, NEW_OBJ(new_pistring("constructor")));
                emit_16u(parser->comp, OP_LOAD_CONST, "constructor", ctor_index);
                emit(parser->comp, OP_GET_MEMBER);
            }

            if (!check(parser, TK_RPAREN))
            {
                if (saw_spread)
                    emit_16u(parser->comp, OP_PUSH_LIST, "", 0);

                do
                {
                    if (match(parser, TK_ELLIPSIS))
                    {
                        if (saw_named)
                        {
                            token_t err = previous(parser);
                            p_errorf(err.line, err.column,
                                     "Positional arguments must come before named arguments.");
                        }

                        expr(parser);
                        emit(parser->comp, OP_LIST_EXTEND);
                    }
                    else if (check(parser, TK_ID) && peek_next(parser).type == TK_ASSIGN)
                    {
                        token_t key_tok = consume(parser, TK_ID, "Expect identifier for named argument.");
                        token_t eq_tok = consume(parser, TK_ASSIGN, "Expect '=' after named argument.");
                        (void)eq_tok;

                        if (!saw_named)
                            saw_named = true;

                        if (named >= 256)
                            p_errorf(key_tok.line, key_tok.column, "Too many named arguments.");

                        char *key = token_value(key_tok);
                        named++;

                        expr(parser); // parse value

                        int index = store_const(parser->comp, NEW_OBJ(new_pistring(key)));
                        emit_16u(parser->comp, OP_LOAD_CONST, key, index);
                    }
                    else
                    {
                        if (saw_named)
                        {
                            token_t err = peek(parser);
                            p_errorf(err.line, err.column,
                                     "Positional arguments must come before named arguments.");
                        }
                        expr(parser);
                        if (saw_spread)
                            emit(parser->comp, OP_LIST_APPEND);
                        else
                            args++;
                    }
                } while (match(parser, TK_COMMA));
            }
            token_t _token = consume(parser, TK_RPAREN, "Expect ')' after function call");
            set_pos(parser, _token);
            char *name = strcmp(token_value(token), ")") == 0 ? "<FUN>" : token_value(token);
            if (saw_spread)
                emit(parser->comp, OP_LIST_FINALIZE);
            if (named > 0)
                emit_16u(parser->comp, OP_PUSH_MAP, "", named);
            if (saw_spread)
            {
                emit_8u(parser->comp, OP_CALL_SPREAD, name, named > 0 ? 1 : 0);
            }
            else if (named > 0)
            {
                // Use OP_CALL_FUNCTION_KW when named arguments are present
                emit_8u(parser->comp, OP_CALL_FUNCTION_KW, name, (uint8_t)args);
            }
            else
            {
                // Use regular OP_CALL_FUNCTION when only positional arguments
                emit_8u(parser->comp, OP_CALL_FUNCTION, name, (uint8_t)args);
            }
        }
        else
            break; // Exit the loop if no member expression is found
    }
}

/**
 * Helper to parse the body of an arrow function
 *
 * This function parses the body of an arrow function, which can be either
 * an expression or a block of code. If the body is a block of code, it
 * emits the bytecode for the block and a final return statement. If the
 * body is an expression, it simply emits the bytecode for the expression.
 *
 * @param parser The parser structure containing the tokens to be parsed.
 */
static void arrow_func(parser_t *parser)
{
    if (match(parser, TK_LBRACE))
    {
        token_t token = previous(parser); // Save position for setting later

        if (check(parser, TK_RBRACE))
        {
            set_pos(parser, token); // Set position at '{' for empty arrow block

            if (is_constructor(parser->comp))
                emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
            else
                emit(parser->comp, OP_PUSH_NIL);

            emit(parser->comp, OP_RETURN);
            parser->is_return = true;
        }
        else
        {
            while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
                declaration(parser);
        }

        if (!parser->is_return)
        {
            token_t token = peek(parser); // Set position before final return

            set_pos(parser, token);

            if (is_constructor(parser->comp))
                emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
            else
                emit(parser->comp, OP_PUSH_NIL);

            emit(parser->comp, OP_RETURN);
            parser->is_return = false;
        }

        token_t rbrace = consume(parser, TK_RBRACE, "Expect '}' after function body.");
        set_pos(parser, rbrace); // Set position at '}'
    }
    else
    {
        token_t token = peek(parser); // Likely the token just before expression
        expr(parser);

        set_pos(parser, token); // Set position of single-expression arrow function
        emit(parser->comp, OP_RETURN);
    }
}
/**
 * primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expr ")" |
 * Parses a primary expression, which could be a literal, a grouped expression,
 * a variable, a list literal, or a map literal. Emits the corresponding bytecode
 * for the parsed expression.
 */
static void primary(parser_t *parser)
{
    // Check for literal values (numbers, strings, boolean, nil)
    if (match_n(parser, 7, TK_NUM, TK_STR, TK_TRUE, TK_FALSE, TK_NIL, TK_INF, TK_NAN))
    {
        token_t token = previous(parser);
        set_pos(parser, token);

        if (token.type == TK_NAN)
            emit_16u(parser->comp, OP_LOAD_CONST, "NAN", 0);
        else if (token.type == TK_INF)
            emit_16u(parser->comp, OP_LOAD_CONST, "INF", 0);
        else
        {
            int index = store_const(parser->comp, new_value(token));
            emit_16u(parser->comp, OP_LOAD_CONST, token_value(token), index);
        }
    }
    // Check for grouped expressions, arrow functions, or tuple literals
    else if (match(parser, TK_LPAREN))
    {
        int _current = parser->current;
        set_pos(parser, previous(parser));

        if (is_lookUp(parser->comp))
        {
            // Lookahead mode: scan past the parenthesized expression without emitting
            // Empty parens () are valid in lookahead — just skip past them
            if (check(parser, TK_RPAREN))
            {
                next(parser); // consume ')'
                // Check if it's an arrow function: () -> ...
                if (match(parser, TK_RARROW))
                {
                    if (match(parser, TK_LBRACE))
                    {
                        int depth = 1;
                        while (depth > 0 && !is_atEnd(parser))
                        {
                            if (check(parser, TK_RBRACE))
                                depth--;
                            else if (check(parser, TK_LBRACE))
                                depth++;
                            if (depth == 0)
                                break;
                            next(parser);
                        }
                        if (depth != 0)
                            p_error("Unmatched '{' in arrow function.", peek(parser).line, peek(parser).column);
                        consume(parser, TK_RBRACE, "Expect '}' after arrow function.");
                    }
                    else
                        expr(parser);
                }
                return;
            }

            // Scan past the contents of the parens
            int depth = 1;
            while (depth > 0 && !is_atEnd(parser))
            {
                if (check(parser, TK_RPAREN))
                    depth--;
                else if (check(parser, TK_LPAREN))
                    depth++;
                next(parser);
            }

            if (depth != 0)
                p_error("Unmatched '(' in grouping expression.", peek(parser).line, peek(parser).column);

            if (match(parser, TK_RARROW))
            {
                if (match(parser, TK_LBRACE))
                {
                    depth = 1;
                    while (depth > 0 && !is_atEnd(parser))
                    {
                        if (check(parser, TK_RBRACE))
                            depth--;
                        else if (check(parser, TK_LBRACE))
                            depth++;
                        if (depth == 0)
                            break;
                        next(parser);
                    }
                    if (depth != 0)
                        p_error("Unmatched '{' in arrow function.", peek(parser).line, peek(parser).column);
                    consume(parser, TK_RBRACE, "Expect '}' after arrow function.");
                }
                else
                    expr(parser);
            }
        }
        else
        {
            // Empty tuple: ()
            if (check(parser, TK_RPAREN))
            {
                consume(parser, TK_RPAREN, "Expect ')' after empty tuple.");

                // Check for arrow function: () -> ...
                if (match(parser, TK_RARROW))
                {
                    bool method_value = parser->object_member;
                    parser->object_member = false;
                    push_function(parser->comp, get_pendingFunctionName(parser));
                    list_t *empty_params = list_create(sizeof(String));
                    parser->comp->current->param_names = empty_params;

                    if (method_value)
                        add_local(parser->comp, "this");
                    add_local(parser->comp, "args");
                    add_local(parser->comp, "kw_args");

                    arrow_func(parser);

                    pop_function(parser->comp, 0);
                    parser->object_member = method_value;
                }
                else
                {
                    // Bare () with no arrow → empty tuple
                    emit_16u(parser->comp, OP_PUSH_TUPLE, "", 0);
                }
                return;
            }

            // Scan ahead to determine whether this is an arrow function
            while (!check(parser, TK_RPAREN))
                next(parser);

            next(parser); // consume ')'

            if (match(parser, TK_RARROW))
            {
                //  Arrow function: (params) -> body
                parser->current = _current;
                list_t *params = param_list(parser);
                int size = list_size(params);
                consume(parser, TK_RPAREN, "Expect ')' after expression.");
                consume(parser, TK_RARROW, "Expect '->' after function parameters.");

                bool method_value = parser->object_member;
                parser->object_member = false;
                push_function(parser->comp, get_pendingFunctionName(parser));
                parser->comp->current->param_names = params;

                if (method_value)
                    add_local(parser->comp, "this");
                for (int i = 0; i < size; i++)
                    add_local(parser->comp, string_get(params, i));
                add_local(parser->comp, "args");
                add_local(parser->comp, "kw_args");

                arrow_func(parser);

                pop_function(parser->comp, size);
                parser->object_member = method_value;
            }
            else
            {
                //  Grouped expression or tuple literal
                parser->current = _current;

                // Parse the first element/expression
                cond_expr(parser);

                if (match(parser, TK_COMMA))
                {
                    // At least one comma seen → tuple literal.
                    // (expr,)  is a single-element tuple.
                    // (expr, expr, ...) is a multi-element tuple.
                    // A trailing comma after the last element is allowed.
                    int size = 1;
                    while (!check(parser, TK_RPAREN) && !is_atEnd(parser))
                    {
                        cond_expr(parser);
                        size++;
                        if (!match(parser, TK_COMMA))
                            break;
                    }
                    consume(parser, TK_RPAREN, "Expect ')' after tuple literal.");
                    emit_16u(parser->comp, OP_PUSH_TUPLE, "", size);
                }
                else
                {
                    //  Grouped expression: (expr) ─
                    consume(parser, TK_RPAREN, "Expect ')' after expression.");
                }
            }
        }
    }
    // Check for variable identifiers
    else if (match(parser, TK_SUPER))
    {
        set_pos(parser, previous(parser));
        emit(parser->comp, OP_LOAD_SUPER);
    }
    // Check for variable identifiers
    else if (match(parser, TK_ID))
    {
        char *name = tk_string(previous(parser));
        set_pos(parser, previous(parser));

        // Lookahead: skip arrow function body without emitting
        if (is_lookUp(parser->comp) && match(parser, TK_RARROW))
        {
            if (match(parser, TK_LBRACE))
            {
                int depth = 1;
                while (depth > 0 && !is_atEnd(parser))
                {
                    if (check(parser, TK_RBRACE))
                        depth--;
                    else if (check(parser, TK_LBRACE))
                        depth++;
                    if (depth == 0)
                        break;
                    next(parser);
                }
                if (depth != 0)
                    p_error("Unmatched '{' in arrow function.", peek(parser).line, peek(parser).column);
                consume(parser, TK_RBRACE, "Expect '}' after arrow function.");
            }
            else
                expr(parser);

            return;
        }

        // Single-param arrow function: name -> body
        if (match(parser, TK_RARROW))
        {
            emit(parser->comp, OP_PUSH_NIL);

            bool method_value = parser->object_member;
            parser->object_member = false;
            push_function(parser->comp, get_pendingFunctionName(parser));
            list_t *single_params = list_create(sizeof(String));
            list_add(single_params, new_string(name));
            parser->comp->current->param_names = single_params;

            if (method_value)
                add_local(parser->comp, "this");
            add_local(parser->comp, name);
            add_local(parser->comp, "args");
            add_local(parser->comp, "kw_args");

            arrow_func(parser);

            pop_function(parser->comp, 1);
            parser->object_member = method_value;
        }
        // Walrus operator: name <- expr
        else if (match(parser, TK_LARROW))
        {
            if (parser->has_walrus)
                p_error("Chained '<-' operators are not allowed",
                        peek(parser).line, peek(parser).column);

            parser->has_walrus = true;
            cond_expr(parser);
            parser->has_walrus = false;

            emit(parser->comp, OP_DUP_TOP);
            store_variable(parser->comp, name);
            return;
        }
        else
        {
            bool is_chained_access = parser->is_store && parser->force_store &&
                                     peek(parser).line == previous(parser).line &&
                                     check_n(parser, 3, TK_DOT, TK_LBRACKET, TK_LPAREN);

            if (is_object(parser->comp) && strcmp(name, "super") == 0)
            {
                emit(parser->comp, OP_LOAD_SUPER);
                return;
            }
            if (!is_chained_access && is_assign(parser))
                store_variable(parser->comp, name);
            else
                load_variable(parser->comp, name);
        }
    }
    // Check for list literals
    else if (match(parser, TK_LBRACKET))
    {
        int size = 0;
        set_pos(parser, previous(parser));

        if (match(parser, TK_RBRACKET))
            emit_16u(parser->comp, OP_PUSH_LIST, "", 0);

        else if (list_isComprehension(parser))
            emit_listComprehension(parser);

        else if (list_hasSpreadItems(parser))
            emit_spreadListLiteral(parser);

        else
        {
            do
            {
                if (!check(parser, TK_RBRACKET))
                {
                    cond_expr(parser);
                    size++;
                }
                else
                    break; // trailing comma
            } while (match(parser, TK_COMMA));
            consume(parser, TK_RBRACKET, "Expect ']' at the end of list literal.");
            emit_16u(parser->comp, OP_PUSH_LIST, "", size);
        }
    }
    // Check for map / set / object literals
    else if (match(parser, TK_LBRACE))
    {
        set_pos(parser, previous(parser));
        if (is_lookUp(parser->comp))
        {
            int depth = 1;
            while (depth > 0 && !is_atEnd(parser))
            {
                if (check(parser, TK_LBRACE))
                    depth++;
                else if (check(parser, TK_RBRACE))
                    depth--;
                next(parser);
                if (depth == 0)
                    break;
            }
            if (depth != 0)
                p_error("Unmatched '}' in map.", peek(parser).line, peek(parser).column);
            return;
        }

        if (match(parser, TK_RBRACE))
        {
            push_object(parser->comp);
            pop_object(parser->comp);
            emit_16u(parser->comp, OP_PUSH_MAP, "", 0);
            emit_mapFinalize(parser);
        }
        else if (!is_mapEntry(parser) && !map_hasSpreadItems(parser))
        {
            emit_setLiteral(parser);
        }
        else
        {
            push_object(parser->comp);
            if (map_hasSpreadItems(parser))
            {
                emit_spreadMapLiteral(parser);
                pop_object(parser->comp);
            }
            else
            {
                char *key;
                int index = 0;
                int size = 0;
                do
                {
                    if (match_n(parser, 5, TK_STR, TK_ID, TK_NUM, TK_FALSE, TK_TRUE))
                    {
                        key = tk_string(previous(parser));
                        index = store_const(parser->comp, NEW_OBJ(new_pistring(key)));
                    }
                    else
                    {
                        p_error("Unexpected key expression.", peek(parser).line, peek(parser).column);
                    }

                    if (match(parser, TK_LPAREN))
                    {
                        // Method shorthand: { key(params) { body } }
                        /**
                         * Parse a function expression as a value in the map.
                         * The function expression is parsed as a lambda function
                         * so it can be used as a value in the map.
                         */
                        list_t *params = param_list(parser);
                        int param_size = list_size(params);
                        consume(parser, TK_RPAREN, "Expect ')' before function body.");
                        consume(parser, TK_LBRACE, "Expect '{' before function body.");

                        push_function(parser->comp, key);
                        parser->comp->current->param_names = params;

                        if (is_object(parser->comp))
                            add_local(parser->comp, "this");

                        for (int i = 0; i < param_size; i++)
                            add_local(parser->comp, string_get(params, i));
                        add_local(parser->comp, "args");
                        add_local(parser->comp, "kw_args");

                        if (match(parser, TK_RBRACE))
                        {
                            if (is_constructor(parser->comp))
                                emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                            else
                                emit(parser->comp, OP_PUSH_NIL);
                            emit(parser->comp, OP_RETURN);
                        }
                        else
                        {
                            while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
                                declaration(parser);

                            if (!parser->is_return)
                            {
                                if (is_constructor(parser->comp))
                                    emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
                                else
                                    emit(parser->comp, OP_PUSH_NIL);
                                emit(parser->comp, OP_RETURN);
                                parser->is_return = false;
                            }
                        }

                        pop_function(parser->comp, param_size);
                        consume(parser, TK_RBRACE, "Expect '}' after function body.");
                    }
                    else
                    {
                        if (strcmp(key, "constructor") == 0)
                            p_error("Constructor is a reserved keyword.", peek(parser).line, peek(parser).column);
                        consume(parser, TK_COLON, "Expect ':' after object key expression.");

                        bool prev_object_member = parser->object_member;
                        char *prev_fun = parser->fun_name;
                        char *prev_obj = parser->object_name;
                        parser->object_member = true;
                        parser->object_name = NULL;

                        if (is_functionLiteral(parser, parser->current))
                            parser->fun_name = key;

                        cond_expr(parser);
                        parser->object_member = prev_object_member;
                        parser->fun_name = prev_fun;
                        parser->object_name = prev_obj;
                    }

                    emit_16u(parser->comp, OP_LOAD_CONST, key, index);
                    size++;
                } while (match(parser, TK_COMMA) && !check(parser, TK_RBRACE));

                consume(parser, TK_RBRACE, "Expect '}' at the end of map literal.");
                pop_object(parser->comp);
                emit_16u(parser->comp, OP_PUSH_MAP, "", size);
                emit_mapFinalize(parser);
            }
        }
    }
    // Anonymous function expressions: fun(params) { body }
    else if (match(parser, TK_FUN))
    {
        set_pos(parser, previous(parser));

        if (is_lookUp(parser->comp))
        {
            int depth = 0;

            consume(parser, TK_LPAREN, "Expect '(' after function name.");

            depth = 1;
            while (depth > 0 && !is_atEnd(parser))
            {
                if (check(parser, TK_LPAREN))
                    depth++;
                else if (check(parser, TK_RPAREN))
                    depth--;
                next(parser);
            }
            if (depth != 0)
                p_error("Unmatched '(' in anonymous function.", peek(parser).line, peek(parser).column);

            consume(parser, TK_LBRACE, "Expect '{' before function body.");

            depth = 1;
            while (depth > 0 && !is_atEnd(parser))
            {
                if (check(parser, TK_LBRACE))
                    depth++;
                else if (check(parser, TK_RBRACE))
                    depth--;
                next(parser);
            }
            if (depth != 0)
                p_error("Unmatched '{' in anonymous function.", peek(parser).line, peek(parser).column);

            return;
        }

        /**
         * Parses an anonymous function expression.
         * Anonymous functions are functions that are declared without a name.
         * They can be used as values in expressions.
         */
        compiler_t *comp = parser->comp;
        // Function expressions do not have their name set until we parse the parameter list, since the name may be needed for recursion within the function body. So we use a placeholder name for now and set the actual name after parsing the parameters.
        consume(parser, TK_LPAREN, "Expect '(' after function name.");
        list_t *params = param_list(parser);
        int size = list_size(params);
        consume(parser, TK_RPAREN, "Expect ')' before function body.");
        consume(parser, TK_LBRACE, "Expect '{' before function body.");

        bool method_value = parser->object_member;
        parser->object_member = false;
        push_function(comp, get_pendingFunctionName(parser));
        comp->current->param_names = params;

        if (method_value)
            add_local(comp, "this");
        // Add the parameters to the local scope
        for (int i = 0; i < size; i++)
            add_local(comp, string_get(params, i));
        add_local(comp, "args"); // Add the "args" variable to the local scope
        add_local(comp, "kw_args");

        if (check(parser, TK_RBRACE))
        {
            // If the anonymous function expression is empty, return nil
            if (is_constructor(comp))
                emit_8u(comp, OP_LOAD_LOCAL, "this", 0);
            else
                emit(comp, OP_PUSH_NIL);
            emit(comp, OP_RETURN);
            parser->is_return = true;
        }
        else
        {
            // Parse the function body
            while (!check(parser, TK_RBRACE) && !is_atEnd(parser))
                declaration(parser);

            if (!parser->is_return)
            {
                // If the anonymous function expression has a return statement
                if (is_constructor(comp))
                    emit_8u(comp, OP_LOAD_LOCAL, "this", 0);
                else
                    emit(comp, OP_PUSH_NIL);
                emit(comp, OP_RETURN);
                parser->is_return = true;
            }
        }

        pop_function(comp, size); // Pop the function from the stack
        parser->object_member = method_value;

        consume(parser, TK_RBRACE, "Expect '}' after function body.");
    }
    else
        p_error("Expect expression.", previous(parser).line, previous(parser).column);
}

/**
 * Frees the memory allocated for the parser.
 * This function releases all resources held by the parser, including tokens,
 * the associated compiler, and the parser structure itself.
 *
 * @param parser the parser object to free
 */
void free_parser(parser_t *parser)
{
    free(parser->tokens); // Free the memory allocated for tokens
    free(parser);         // Free the parser structure itself
}
