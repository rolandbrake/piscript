#ifndef PI_COMPILER_H
#define PI_COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pi_stack.h"
#include "pi_opcode.h"
#include "pi_value.h"
#include "common.h"
#include "list.h"
#include "pi_table.h"

// Represents a local variable in the current scope
typedef struct
{
    char *name;       // Variable name
    int depth;        // Scope depth where the variable is declared
    bool is_captured; // Indicates if the variable is captured as an upvalue
} local_t;

// Represents the compilation context for functions and blocks
typedef struct
{
    bool is_function; // Indicates if this context is for a function
    char *fun_name;   // Name of the function (if applicable)
    list_t *code;     // PiList of bytecode instructions
    list_t *instrs;   // PiList of instruction metadata (e.g., debug info)
    list_t *upvalues; // PiList of upvalues used in the function
    stack_t *locals;  // Stack of local variables
    int depth;        // Current scope depth
} context_t;

// Represents a loop structure to track break/continue handling
typedef struct
{
    int _continue;   // Address to jump to when 'continue' is encountered
    int depth;       // Scope depth of the loop
    stack_t *breaks; // Stack of break statement addresses
    bool is_for;     // Flag indicating if the loop is a for-loop
} loop_t;

// Compiler structure that maintains the current state of compilation
typedef struct
{
    list_t *code;      // PiList of bytecode instructions
    list_t *constants; // PiList of constant values

    list_t *names;         // PiList of variable names
    list_t *builtin_names; // PiList of built-in names

    stack_t *locals;    // Stack of local variables
    stack_t *contexts;  // Stack of active compilation contexts
    context_t *current; // Pointer to the current active context
    stack_t *loops;     // Stack of active loops
    table_t *instrs;    // Table to store instruction metadata
    stack_t *objects;   // Stack of objects being allocated

    bool is_lookUp;  // Flag for lookup operations
    bool is_upvalue; // Flag indicating if a variable is an upvalue
    bool is_repl; // Flag indicating if the compiler is in REPL mode

    int current_line; // Current line number in the source code
    int current_col;  // Current column number in the source code

    char *name; // Name of the current variable being processed
} compiler_t;

// Represents an upvalue (captured variable from an outer scope)
typedef struct
{
    bool is_local; // Indicates if the upvalue comes from a local variable
    int index;     // Index of the captured variable
} upvalue_t;

// Represents metadata for an instruction (used for debugging)
typedef struct
{
    char *descr; // Description of the instruction
    int line;    // Line number in the source code
    int column;  // Column number in the source code
    int offset;  // Bytecode offset of the instruction

    char *fun_name; // Name of the function this instruction belongs to (NULL for global)
    OpCode opcode;  // Actual opcode value

    int num_operands;  // How many operands this instruction has
    uint8_t *operands; // Dynamically allocated array of operands

} instr_t;

// Function to initialize a new compiler instance
compiler_t *init_compiler();

// Stores a constant value in the compiler's constant pool
int store_const(compiler_t *comp, Value value);

// Returns the index of a variable name in the compiler's name table
int name_index(compiler_t *comp, char *name);
int store_name(compiler_t *comp, char *name);

// Adds a new local variable to the current scope
void add_local(compiler_t *comp, char *name);
bool is_localScope(compiler_t *comp);

// Functions for managing object allocation and scope
void push_object(compiler_t *comp);
void pop_object(compiler_t *comp);
bool is_object(compiler_t *comp);
bool is_constructor(compiler_t *comp);

// Functions related to variable lookup and resolution
bool is_lookUp(compiler_t *comp);
bool look_up(compiler_t *comp, bool value);

// Bytecode emission functions
int emit(compiler_t *comp, OpCode opcode);
int emit_8u(compiler_t *comp, OpCode opcode, char *descr, int operand);
int emit_16u(compiler_t *comp, OpCode opcode, char *descr, int operand);

// Emits a pop instruction to remove values from the stack
int emit_pop(compiler_t *comp, int depth);

// Emits a jump instruction and later patches its address
int emit_jump(compiler_t *comp, int address);
void patch_jump(compiler_t *comp, int address);

// Functions for managing local variables
void remove_locals(compiler_t *comp, int size);
int get_local(compiler_t *comp, char *name);
int get_localSize(compiler_t *comp, int depth);
int resolve_local(compiler_t *comp, int depth, char *name);

// Functions for handling upvalues (captured variables from outer scopes)
int resolve_upvalue(compiler_t *comp, int depth, char *name);
int add_upvalue(compiler_t *comp, int depth, int index, bool is_local);

// Functions for managing variable declarations and assignments
void add_variable(compiler_t *comp, char *name);
void store_variable(compiler_t *comp, char *name);
void load_variable(compiler_t *comp, char *name);

// Functions for managing scope tracking
void push_scope(compiler_t *comp);
void pop_scope(compiler_t *comp);

// Functions for handling loops (break/continue support)
void push_loop(compiler_t *comp, int address, bool is_for);
void pop_loop(compiler_t *comp, int address);
bool is_forLoop(compiler_t *comp);
bool in_loop(compiler_t *comp);
int loop_depth(compiler_t *comp);

// Functions for handling function definitions
void push_function(compiler_t *comp, char *name);
void pop_function(compiler_t *comp, int params);

// Functions for handling break/continue statements in loops
void push_break(compiler_t *comp, int address);
int get_continue(compiler_t *comp);

// Functions for managing and retrieving bytecode
void add_code(compiler_t *comp, byte _byte);
int code_size(compiler_t *comp);




// Error handling functions
void p_error(const char *message, int line, int column);
void p_errorf(int line, int column, const char *format, ...);

// Debugging and memory management functions
void dis(compiler_t *comp);
void free_compiler(compiler_t *comp);

// Prints the list of currently active local variables (for debugging)
void print_locals(compiler_t *comp);

// Resets the compiler to its initial state for reuse
void reset_compiler(compiler_t *comp);

#endif
