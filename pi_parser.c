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
#include "string.h"

char *comp_ops[] = {"==", "!=", ">", "<", ">=", "<=", "in"};
char *bin_ops[] = {"+", "-", "*", "/", "%", "&&", "||", "**", "&", "|", "^", "<<", ">>", ">>>", ".", "is"};
char *unary_ops[] = {"+", "-", "!", "~", "#", "++", "--", "typeof"};

// Function prototypes for static functions
static void program(parser_t *parser);
static void declaration(parser_t *parser);
static void var_decl(parser_t *parser);
static void func_decl(parser_t *parser);
static void statement(parser_t *parser);
static void expr_state(parser_t *parser);
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
    if (parser->is_store && check_n(parser, 11, TK_ASSIGN, TK_PLUS_ASSIGN, TK_MINUS_ASSIGN, TK_DIV_ASSIGN, TK_MULT_ASSIGN,
                                    TK_MOD_ASSIGN, TK_BITOR_ASSIGN, TK_XOR_ASSIGN, TK_BITAND_ASSIGN, TK_INCR, TK_DECR))
    {
        parser->is_store = false; // Reset the store state
        return true;              // Return true as the token is an assignment operator
    }
    return false; // Return false if no assignment operator is found
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
    parser->is_return = false;
    parser->has_walrus = false;

    // Initialize the compiler associated with the parser
    parser->comp = comp;

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
        // Collect global variable declarations
        else if (match(parser, TK_LET))
        {
            int start = parser->current - 1; // Start at 'let'
            var_decl(parser);                // Parse variable declaration
            int end = parser->current;
            mark_tokens(parser, start, end); // Mark tokens as processed
        }
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
            statement(parser); // Parse remaining statements
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
    // If not a variable or function declaration, parse as a statement
    else
        statement(parser); // Parse as a statement
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
        assignment(parser, true);

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

    if (is_object(parser->comp))
        emit(parser->comp, OP_PUSH_NIL);

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

        // Add parameters as locals
        for (int i = 0; i < size; i++)
            add_local(parser->comp, string_get(params, i));
        add_local(parser->comp, "args");

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
            // ⬇️ Important: Mark where the implicit return comes from
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
            // ⬇️ Mark function definition location before storing it
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
/**
 * statement -> block | if_stmt | while_stmt | for_stmt | break_stmt | continue_stmt | return_stmt | expr_state
 * Parses a statement, which is a single expression or a block of expressions.
 * @param parser The parser object used for parsing.
 */
static void statement(parser_t *parser)
{
    if (match(parser, TK_LBRACE))
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
    else
        expr_state(parser);
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
    emit_jump(parser->comp, address);

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
        emit_8u(parser->comp, OP_LOAD_LOCAL, "this", 0);
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
    if (token.type >= TK_ASSIGN && token.type <= TK_MOD_ASSIGN)
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
            cond_expr(parser);

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
    dot_expr(parser); // 👈 changed from mult_expr()
    while (match_n(parser, 2, TK_PLUS, TK_MINUS))
    {

        token_t op = previous(parser);
        dot_expr(parser); // 👈 changed from mult_expr()
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
 * unary_expr -> ("+" | "-" | "!" | "~" | "#" | "++" | "--" | "typeof")? member_expr
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

        // Pre-increment / Pre-decrement or other unary ops
        current = parser->current;
        member_expr(parser);
        set_pos(parser, op_token);

        if (op == TK_INCR || op == TK_DECR)
        {
            token_t target = previous(parser);

            // Disallow applying ++ or -- to literals
            if (target.type == TK_NUM || target.type == TK_STR || target.type == TK_TRUE ||
                target.type == TK_FALSE || target.type == TK_NIL)
                p_error("Increment/Decrement operations cannot be applied to calls or literals.",
                        target.line, target.column);

            int type = (op == TK_INCR) ? 5 : 6;
            emit_8u(parser->comp, OP_UNARY, unary_ops[type], type);
            emit(parser->comp, OP_DUP_TOP);

            parser->current = current;
            parser->is_store = true;
            member_expr(parser);
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
    token_t token = peek(parser); // position marking for error reporting

    // Handle the start of the slice
    if (check(parser, TK_COLON))
    {
        // If the start is missing, assume 0
        index = store_const(parser->comp, NEW_NUM(0));
        emit_16u(parser->comp, OP_LOAD_CONST, "0", index);
        is_slice = true;
    }
    else
        cond_expr(parser); // Parse start expression

    // Check for the first colon (start:end)
    if (match(parser, TK_COLON))
    {
        is_slice = true;
        token = previous(parser);

        // Handle the end expression
        if (!check(parser, TK_RBRACKET) && !check(parser, TK_COLON))
            cond_expr(parser);
        else
        {
            // If the end is missing, assume infinity
            emit_16u(parser->comp, OP_LOAD_CONST, "inf", 1);
        }

        // Check for the second colon (start:end:step)
        if (match(parser, TK_COLON))
        {
            // Handle the step expression
            if (!check(parser, TK_RBRACKET))
                cond_expr(parser);
            else
            {
                // If the step is missing, assume 1
                index = store_const(parser->comp, NEW_NUM(1));
                emit_16u(parser->comp, OP_LOAD_CONST, "1", index);
            }
        }
        else
        {
            // If step colon is missing, assume 1
            index = store_const(parser->comp, NEW_NUM(1));
            emit_16u(parser->comp, OP_LOAD_CONST, "1", index);
        }

        set_pos(parser, token); // Set the position to the start of the slice
        // Emit the slice operation
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

        set_pos(parser, token);
        if (match(parser, TK_DOT))
        {
            token_t token = previous(parser); // For position tracking
            // Handle property access using dot notation
            token_t name = consume(parser, TK_ID, "Expect property name after '.'");

            int index = store_const(parser->comp, new_value(name)); // Store the property name as a constant
            emit_16u(parser->comp, OP_LOAD_CONST, token_value(name), index);

            if (is_assign(parser))
                emit(parser->comp, OP_SET_ITEM); // Emit bytecode to set the property value
            else
                emit(parser->comp, OP_GET_ITEM); // Emit bytecode to get the property value
        }
        else if (match(parser, TK_LBRACKET))
        {
            token_t token = previous(parser);
            bool is_slice = slice_expr(parser); // Parse the index expression
            consume(parser, TK_RBRACKET, "Expect ']' after list index expression");

            if (is_slice && is_assign(parser))
                p_error("Cannot assign to slice", peek(parser).line, peek(parser).column);

            if (!is_slice)
            {
                bool assign = is_assign(parser);
                emit(parser->comp, assign ? OP_SET_ITEM : OP_GET_ITEM);
            }
        }

        // handle function call
        else if (match(parser, TK_LPAREN))
        {
            int args = 0;
            // Handle function call
            // Call expression logic goes here
            if (!check(parser, TK_RPAREN))
            {
                expr(parser);
                args++;
                while (match(parser, TK_COMMA))
                {
                    expr(parser);
                    args++;
                }
            }
            token_t _token = consume(parser, TK_RPAREN, "Expect ')' after function call");
            set_pos(parser, _token);
            char *name = strcmp(token_value(token), ")") == 0 ? "<FUN>" : token_value(token);
            emit_8u(parser->comp, OP_CALL_FUNCTION, name, (byte)args);
        }
        else
            break; // Exit the loop if no member expression is found
    }
}

// Helper to parse the body of an arrow functionstatic void arrow_func(parser_t *parser)
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
            // Emit bytecode to load the constant value
            emit_16u(parser->comp, OP_LOAD_CONST, token_value(token), index);
        }
    }
    // Check for grouped expressions
    else if (match(parser, TK_LPAREN))
    {
        int _current = parser->current;
        set_pos(parser, previous(parser));

        if (is_lookUp(parser->comp))
        {
            int depth = 1;
            while (depth > 0 && !is_atEnd(parser))
            {
                if (check(parser, TK_RPAREN))
                    depth--;
                else if (check(parser, TK_LPAREN))
                    depth++;

                next(parser); // Move to the next token
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

                        next(parser); // Move to the next token
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
            while (!check(parser, TK_RPAREN))
                next(parser);

            next(parser);
            // handle arrow functions
            if (match(parser, TK_RARROW))
            {
                parser->current = _current;
                list_t *params = param_list(parser);
                int size = list_size(params);
                consume(parser, TK_RPAREN, "Expect ')' after expression.");
                consume(parser, TK_RARROW, "Expect '->' after function parameters.");

                push_function(parser->comp, NULL);

                if (is_object(parser->comp))
                    add_local(parser->comp, "this");

                for (int i = 0; i < size; i++)
                    add_local(parser->comp, string_get(params, i));
                add_local(parser->comp, "args");

                arrow_func(parser);

                pop_function(parser->comp, size + (is_object(parser->comp) ? 1 : 0));
            }
            else
            {
                parser->current = _current;
                // Parse the expression inside parentheses
                assignment(parser, true); // Parse the inner expression
                consume(parser, TK_RPAREN, "Expect ')' after expression.");
            }
        }
    }
    // Check for variable identifiers
    else if (match(parser, TK_ID))
    {
        char *name = tk_string(previous(parser));
        set_pos(parser, previous(parser));

        // Lookup for right-associative arrow functions or assignment chains
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

                    next(parser); // Move to the next token
                }

                if (depth != 0)
                    p_error("Unmatched '{' in arrow function.", peek(parser).line, peek(parser).column);
                consume(parser, TK_RBRACE, "Expect '}' after arrow function.");
            }
            else
                expr(parser);

            return;
        }

        // handle arrow functions
        if (match(parser, TK_RARROW))
        {

            emit(parser->comp, OP_PUSH_NIL);

            if (is_object(parser->comp))
                emit(parser->comp, OP_PUSH_NIL);

            push_function(parser->comp, NULL);

            if (is_object(parser->comp))
                add_local(parser->comp, "this");

            add_local(parser->comp, name);
            add_local(parser->comp, "args");

            arrow_func(parser);

            pop_function(parser->comp, (is_object(parser->comp) ? 2 : 1));
        }
        // First handle potential walrus operator
        else if (match(parser, TK_LARROW))
        {

            // Verify we didn't chain assignments
            if (parser->has_walrus)
                p_error("Chained '<-' operators are not allowed",
                        peek(parser).line, peek(parser).column);

            parser->has_walrus = true;
            // Parse the right-hand side (disallow chaining)
            cond_expr(parser); // Use expr() instead of cond_expr() to prevent chaining

            parser->has_walrus = false;

            // Duplicate result on stack (if needed by your VM)
            emit(parser->comp, OP_DUP_TOP);

            // Emit store instruction for lhs variable
            store_variable(parser->comp, name);

            return;
        }
        else
        {
            if (is_assign(parser))
                store_variable(parser->comp, name); // Handle variable assignment
            else                                    // Load variable value
                load_variable(parser->comp, name);
        }
    }
    // Check for list literals
    else if (match(parser, TK_LBRACKET))
    {
        int size = 0;
        set_pos(parser, previous(parser));
        if (match(parser, TK_RBRACKET))
            emit_16u(parser->comp, OP_PUSH_LIST, "", 0); // Emit empty list
        else
        {
            do
            {
                if (!check(parser, TK_RBRACKET))
                {
                    cond_expr(parser); // Parse list elements
                    size++;
                }
                else
                    // Handle trailing comma case
                    break;

            } while (match(parser, TK_COMMA));
            consume(parser, TK_RBRACKET, "Expect ']' at the end of list literal.");
            emit_16u(parser->comp, OP_PUSH_LIST, "", size); // Emit list with elements
        }
    }
    // Check for map / object literals
    else if (match(parser, TK_LBRACE))
    {
        set_pos(parser, previous(parser));
        if (is_lookUp(parser->comp))
        {
            // Verify the map is a valid expression
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

        token_t token = previous(parser);
        int size = 0;

        push_object(parser->comp);
        if (match(parser, TK_RBRACE))
        {
            pop_object(parser->comp);
            emit_16u(parser->comp, OP_PUSH_MAP, "", 0); // Emit empty map
        }
        else
        {
            char *key;
            int index = 0;
            do
            {
                if (match_n(parser, 5, TK_STR, TK_ID, TK_NUM, TK_FALSE, TK_TRUE))
                {
                    key = tk_string(previous(parser));
                    index = store_const(parser->comp, NEW_OBJ(new_pistring(key)));
                }
                else
                    p_error("Unexpected key expression.", peek(parser).line, peek(parser).column);

                if (match(parser, TK_LPAREN))
                {
                    /**
                     * Parse a function expression as a value in the map.
                     * The function expression is parsed as a lambda function
                     * so it can be used as a value in the map.
                     */
                    list_t *params = param_list(parser);
                    int size = list_size(params);
                    consume(parser, TK_RPAREN, "Expect ')' before function body.");
                    consume(parser, TK_LBRACE, "Expect '{' before function body.");

                    push_function(parser->comp, key);

                    if (is_object(parser->comp))
                        add_local(parser->comp, "this");

                    for (int i = 0; i < size; i++)
                        add_local(parser->comp, string_get(params, i));
                    add_local(parser->comp, "args");

                    // if (check(parser, TK_RBRACE))
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
                        // Parse the function body
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

                    pop_function(parser->comp, size + (is_object(parser->comp) ? 1 : 0));
                    consume(parser, TK_RBRACE, "Expect '}' after function body.");
                }
                else
                {
                    if (strcmp(key, "constructor") == 0)
                        p_error("Constructor is a reserved keyword.", peek(parser).line, peek(parser).column);
                    consume(parser, TK_COLON, "Expect ':' after object key expression.");
                    cond_expr(parser);
                }

                // Emit bytecode to load the key as a constant
                emit_16u(parser->comp, OP_LOAD_CONST, key, index);
                size++;
            } while (match(parser, TK_COMMA) && !check(parser, TK_RBRACE)); // Allow trailing comma

            consume(parser, TK_RBRACE, "Expect '}' at the end of map literal.");
            pop_object(parser->comp);
            emit_16u(parser->comp, OP_PUSH_MAP, "", size); // Emit map with key-value pairs
        }
    }

    // Parse anonymous function expressions
    else if (match(parser, TK_FUN))
    {
        set_pos(parser, previous(parser));
        /**
         * Parses an anonymous function expression.
         * Anonymous functions are functions that are declared without a name.
         * They can be used as values in expressions.
         */
        compiler_t *comp = parser->comp;

        // Function expressions
        consume(parser, TK_LPAREN, "Expect '(' after function name.");
        list_t *params = param_list(parser); // Parse the parameter list
        int size = list_size(params);        // Get the number of parameters
        consume(parser, TK_RPAREN, "Expect ')' before function body.");
        consume(parser, TK_LBRACE, "Expect '{' before function body.");

        push_function(comp, NULL); // Push the function onto the stack

        if (is_object(comp))
            add_local(comp, "this"); // Add the "this" variable to the local scope

        // Add the parameters to the local scope
        for (int i = 0; i < size; i++)
            add_local(comp, string_get(params, i));
        add_local(comp, "args"); // Add the "args" variable to the local scope

        if (match(parser, TK_RBRACE))
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

        pop_function(comp, size + (is_object(parser->comp) ? 1 : 0)); // Pop the function from the stack

        consume(parser, TK_RBRACE, "Expect '}' after function body.");
    }
    else
        p_error("Expect expression.", previous(parser).line, previous(parser).column); // Handle unexpected tokens
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
    free(parser->tokens);        // Free the memory allocated for tokens
    // free_compiler(parser->comp); // Free resources associated with the compiler
    free(parser);                // Free the parser structure itself
}
