#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "pi_compiler.h"
#include "pi_object.h"
#include "pi_opcode.h"
#include "pi_stack.h"
#include "common.h"
#include "list.h"
#include "string.h"

static const char *op_names[] = {
    [0x4] = "RETURN_VALUE",
    [0x5] = "LOAD_CONST",
    [0x6] = "PRINT_VALUE",
    [0x7] = "POP_TOP",
    [0x8] = "PUSH_STACK",
    [0x9] = "PUSH_NIL",
    [0xa] = "STORE_GLOBAL",
    [0xb] = "LOAD_GLOBAL",
    [0xc] = "STORE_LOCAL",
    [0xd] = "LOAD_LOCAL",
    [0xe] = "JUMP",
    [0xf] = "JUMP_IF_FALSE",
    [0x11] = "CALL_FUNCTION",
    [0x12] = "POP_N",
    [0x13] = "COMPARE_OP",
    [0x14] = "JUMP_IF_TRUE",
    [0x15] = "HALT_OP",
    [0x16] = "PUSH_ITER",
    [0x17] = "LOOP_OP",
    [0x18] = "PUSH_RANGE",
    [0x19] = "BINARY_OP",
    [0x1a] = "PUSH_LIST",
    [0x1b] = "STORE_UPVALUE",
    [0x1c] = "LOAD_UPVALUE",
    [0x1d] = "NO_OP",
    [0x1e] = "CREATE_UPVALUE",
    [0x21] = "PUSH_FUNCTION",
    [0x22] = "PUSH_MAP",
    [0x23] = "PUSH_UPVALUE",
    [0x24] = "PUSH_CLOSURE",
    [0x25] = "DUP_TOP",
    [0x26] = "PUSH_SLICE",
    [0x27] = "GET_ITEM",
    [0x28] = "SET_ITEM",
    [0x29] = "UNARY_OP",
    [0x2a] = "DEBUG_OP",
    [0x2b] = "POP_ITER",
    [0x3c] = "CLOSE_UPVALUE",
};

/**
 * Create a new context for the compiler.
 *
 * @param[in] is_function Whether or not the context is a function.
 * @param[in] code The list of instructions for the context.
 * @param[in] fun_name The name of the function, if applicable.
 *
 * @return A pointer to the new context.
 */
static context_t *create_context(bool is_function, list_t *code, char *fun_name)
{
    static int f_count = 0;
    context_t *context = malloc(sizeof(context_t));

    context->upvalues = list_create(sizeof(upvalue_t));
    context->locals = stack_create(sizeof(local_t));
    // context->instrs = list_create(sizeof(String));
    context->instrs = list_create(sizeof(instr_t));

    context->is_function = is_function;
    context->depth = 0;
    context->code = code;

    if (fun_name == NULL && is_function)
    {
        context->fun_name = malloc(32); // Adjust size as needed
        sprintf(context->fun_name, "function: #%d", f_count);
        f_count++;
    }
    else
        context->fun_name = fun_name;

    return context;
}

static context_t *current_context(compiler_t *comp)
{
    return (context_t *)top(comp->contexts);
}

static upvalue_t *create_upvalue(int index, bool is_local)
{
    upvalue_t *upvalue = malloc(sizeof(upvalue_t));

    upvalue->index = index;
    upvalue->is_local = is_local;
    return upvalue;
}

compiler_t *init_compiler()
{

    compiler_t *comp = (compiler_t *)malloc(sizeof(compiler_t));

    // Initialize list_t members
    comp->code = list_create(sizeof(uint8_t));
    comp->constants = list_create(sizeof(Value));

    list_add(comp->constants, &NEW_NUM(NAN));
    list_add(comp->constants, &NEW_NUM(INFINITY));

    list_add(comp->constants, &NEW_BOOL(true));
    list_add(comp->constants, &NEW_BOOL(false));

    // names for storing the names of the global variables
    comp->names = list_create(sizeof(String));

    // Initialize stack_t members
    comp->locals = stack_create(sizeof(local_t));
    comp->contexts = stack_create(sizeof(context_t));
    comp->loops = stack_create(sizeof(loop_t));
    comp->objects = stack_create(sizeof(String));
    comp->name = "";

    // Initialize the current context
    comp->current = create_context(false, comp->code, NULL);

    comp->instrs = comp->current->instrs;

    comp->is_lookUp = false;
    comp->is_upvalue = false;

    push(comp->contexts, comp->current);

    return comp;
}

static int read_short(compiler_t *comp, int index)
{
    uint8_t *code = (uint8_t *)comp->code->data; // Access the bytecode from the VM's code
    int high = code[index] & 0xFF;               // Get the high byte and mask it
    int low = code[index + 1] & 0xFF;            // Get the low byte and mask it

    return (high << 8) | low; // Combine high and low bytes into a 16-bit short
}

void add_code(compiler_t *comp, byte _byte)
{

    // Ensure the list pointer comp and its code member are valid
    if (!comp || !comp->code)
    {
        fprintf(stderr, "Invalid compiler or code list\n");
        exit(EXIT_FAILURE);
    }

    int size = comp->code->size;
    int cap = comp->code->capacity;
    if (size == cap)
    {
        cap = cap == 0 ? 8 : cap * 2;
        comp->code = realloc(comp->code, cap * sizeof(byte));
        if (comp->code->data == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    // Add the byte to the code array
    ((byte *)comp->code->data)[size++] = _byte;

    comp->code->size = size;
    comp->code->capacity = cap;
}

bool is_localScope(compiler_t *comp)
{
    return comp->current->depth > 0 || comp->current->is_function;
}

void push_object(compiler_t *comp)
{
    if (!comp->is_lookUp)
        push(comp->objects, new_string(comp->name));
}

void pop_object(compiler_t *comp)
{
    if (!comp->is_lookUp)
        pop(comp->objects);
}

bool is_object(compiler_t *comp)
{
    return !is_empty(comp->objects);
}

bool is_constructor(compiler_t *comp)
{
    if (is_object(comp) && comp->current->is_function)
        return strcmp(comp->current->fun_name, "constructor") == 0;

    return false;
}
bool is_lookUp(compiler_t *comp)
{
    return comp->is_lookUp;
}

bool look_up(compiler_t *comp, bool value)
{
    bool look_up = comp->is_lookUp;
    comp->is_lookUp = value;
    return look_up;
}

void print_locals(compiler_t *comp)
{
    printf("Locals stack (top to bottom):\n");

    // Iterate from top to bottom of the stack.
    local_t *local = (local_t *)top(comp->current->locals);
    printf("Local: name = %s, depth = %d, is_captured = %s\n",
           local->name,
           local->depth,
           local->is_captured ? "true" : "false");

    printf("\n");
}

void add_local(compiler_t *comp, char *name)
{
    local_t *local = malloc(sizeof(local_t));
    local->name = strdup(name); // Allocate and copy name string
    local->depth = comp->current->depth;
    local->is_captured = false;

    push(comp->current->locals, local);
}

int get_local(compiler_t *comp, char *name)
{
    int index = -1;
    int depth = stack_size(comp->contexts) - 1;
    comp->is_upvalue = false;

    index = resolve_local(comp, depth, name);
    if (index != -1)
        return index;
    else
    {
        index = resolve_upvalue(comp, depth, name);
        comp->is_upvalue = true;
    }
    return index;
}

int get_localSize(compiler_t *comp, int depth)
{
    int size = 0;

    stack_t *locals = comp->current->locals;
    int l_size = stack_size(locals);

    for (int i = l_size - 1; i >= 0; i--)
    {
        local_t *local = (local_t *)stack_getAt(locals, i);
        if (local->depth >= depth)
            size++;
        else
            break;
    }
    return size;
}

int resolve_local(compiler_t *comp, int depth, char *name)
{
    int index = -1;
    local_t *local;
    context_t *context = stack_getAt(comp->contexts, depth);
    for (int i = stack_size(context->locals) - 1; i >= 0; i--)
    {
        local = (local_t *)stack_getAt(context->locals, i);
        if (strcmp(local->name, name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

int resolve_upvalue(compiler_t *comp, int depth, char *name)
{
    if (depth == 0)
        return -1;
    int index = resolve_local(comp, depth - 1, name);
    if (index != -1)
        return add_upvalue(comp, depth, index, true);
    int upvalue = resolve_upvalue(comp, depth - 1, name);
    if (upvalue != -1)
        return add_upvalue(comp, depth, upvalue, false);
    return -1;
}

int add_upvalue(compiler_t *comp, int depth, int index, bool is_local)
{
    context_t *current = stack_getAt(comp->contexts, depth);
    upvalue_t *upvalue = malloc(sizeof(upvalue_t));
    int size = list_size(current->upvalues);
    for (int i = 0; i < size; i++)
    {
        upvalue_t *_upvalue = (upvalue_t *)list_getAt(current->upvalues, i);
        if (_upvalue->index == index && _upvalue->is_local == is_local)
            return i;
    }
    list_add(current->upvalues, create_upvalue(index, is_local));
    return size - 1;
}

void add_variable(compiler_t *comp, char *name)
{
    int g_index = -1;
    if (is_localScope(comp))
        add_local(comp, name);
    else
    {
        g_index = store_name(comp, name);
        emit_8u(comp, OP_STORE_GLOBAL, name, g_index);
    }
}

/**
 * Stores a variable in the current scope.
 * If the variable is local, it checks if the variable is already declared.
 * If the variable is global, it stores the variable in the global scope.
 * @param comp The compiler instance.
 * @param name The name of the variable to store.
 */
void store_variable(compiler_t *comp, char *name)
{
    // If the variable is local, check if the variable is already declared
    if (is_localScope(comp))
    {
        int index = get_local(comp, name);
        if (index != -1)
        {
            // Store the variable in the local scope
            emit_8u(comp, comp->is_upvalue ? OP_STORE_UPVALUE : OP_STORE_LOCAL, name, index);
        }
        else
        {
            // Store the variable in the global scope
            int g_index = store_name(comp, name);
            emit_8u(comp, OP_STORE_GLOBAL, name, g_index);
        }
    }
    else
    {
        // Store the variable in the global scope
        int g_index = store_name(comp, name);
        emit_8u(comp, OP_STORE_GLOBAL, name, g_index);
    }
}

void load_variable(compiler_t *comp, char *name)
{
    int index = get_local(comp, name);
    if (index != -1)
    {
        if (comp->is_upvalue)
            emit_8u(comp, OP_LOAD_UPVALUE, name, index);
        else
            emit_8u(comp, OP_LOAD_LOCAL, name, index);
    }
    else
    {
        int g_index = name_index(comp, name);
        if (g_index == -1)
            g_index = store_name(comp, name);
        emit_8u(comp, OP_LOAD_GLOBAL, name, g_index);
    }
}

/**
 * Finds the index of a given name in the list of names stored in the compiler.
 *
 * @param comp A pointer to the compiler instance containing the list of names.
 * @param name The name to search for in the list.
 * @return The index of the name if found, or -1 if not found.
 */
int name_index(compiler_t *comp, char *name)
{
    // Iterate through the list of names in the compiler
    for (int i = 0; i < comp->names->size; i++)
    {
        char *_name = string_get(comp->names, i);
        // Compare the current name with the target name
        if (strcmp(_name, name) == 0)
            return i; // Return the index if a match is found
    }
    return -1; // Return -1 if the name is not found in the list
}

int store_name(compiler_t *comp, char *name)
{
    int index = name_index(comp, name);
    if (index != -1)
        return index; // Name already exists, return the index

    list_add(comp->names, new_string(name));

    return comp->names->size - 1;
}

/**
 * Removes a specified number of local variables from the current context's local stack.
 * This function is used to manage scope by clearing locals when a scope is exited.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 * @param size The number of local variables to remove from the stack.
 */
void remove_locals(compiler_t *comp, int size)
{
    // Pop the specified number of local variables from the stack
    while (size-- > 0)
        pop(comp->current->locals);
}

/**
 * Increments the depth of the current scope in the compiler.
 * This function is used to manage scope levels by increasing
 * the depth whenever a new scope is entered.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 */
void push_scope(compiler_t *comp)
{
    // Increment the depth of the current scope
    comp->current->depth++;
}

/**
 * Decrements the depth of the current scope in the compiler.
 * This function is used to manage scope levels by decreasing
 * the depth whenever a scope is exited.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 */
void pop_scope(compiler_t *comp)
{
    // Get the number of local variables at the current depth
    int size = emit_pop(comp, comp->current->depth);

    // Remove the local variables from the stack
    remove_locals(comp, size);

    // Decrement the depth of the current scope
    comp->current->depth--;
}

/**
 * Push a new loop context onto the loops stack.
 * This function is used to create and initialize a new loop context.
 *
 * @param comp A pointer to the compiler instance.
 * @param address The address to jump to for continuing the loop.
 * @param is_for A boolean indicating if the loop is a for-loop.
 */
void push_loop(compiler_t *comp, int address, bool is_for)
{
    loop_t *loop = (loop_t *)malloc(sizeof(loop_t)); // Allocate memory for a new loop

    loop->_continue = address;                // Set the continue address
    loop->depth = comp->current->depth;       // Set the loop depth based on the current scope depth
    loop->breaks = stack_create(sizeof(int)); // Create a stack to hold break addresses
    loop->is_for = is_for;                    // Indicate if the loop is a for-loop

    push(comp->loops, loop); // Push the new loop onto the stack
}

/**
 * Pops the current loop context from the loops stack and
 * patches the break instructions to jump to the given address.
 *
 * @param comp A pointer to the compiler instance.
 * @param address The address to jump to for continuing the loop.
 */
void pop_loop(compiler_t *comp, int address)
{
    loop_t *loop = (loop_t *)pop(comp->loops); // Pop the current loop from the stack
    stack_t *breaks = loop->breaks;            // Get the stack of break addresses

    int16_t offset = address - comp->code->size; // Calculate the offset to the continue address

    // Emit a jump instruction to the continue address
    emit_16u(comp, OP_JUMP, "", offset);

    // Patch the break instructions to jump to the continue address
    while (is_empty(breaks) == false)      // While there are still break addresses
        patch_jump(comp, POP_INT(breaks)); // Patch the jump to the continue address

    // Free the memory allocated for the stack of break addresses
    stack_free(loop->breaks);

    // Free the memory allocated for the loop context
    free(loop);
}

/**
 * Pushes the given address onto the stack of break addresses for the current loop.
 *
 * @param comp A pointer to the compiler instance.
 * @param address The address to push onto the stack of break addresses.
 */
void push_break(compiler_t *comp, int address)
{
    /*
     * Push the address onto the stack of break addresses
     * for the current loop.
     */
    PUSH_INT(((loop_t *)top(comp->loops))->breaks, address);
}

/**
 * Gets the address to jump to for continuing the current loop.
 *
 * @param comp A pointer to the compiler instance.
 * @return The address to jump to for continuing the current loop.
 */
int get_continue(compiler_t *comp)
{
    return ((loop_t *)top(comp->loops))->_continue;
}
/**
 * Checks if the current loop is a for-loop.
 *
 * @param comp A pointer to the compiler instance.
 * @return true if the current loop is a for-loop, false otherwise.
 */
bool is_forLoop(compiler_t *comp)
{
    return ((loop_t *)top(comp->loops))->is_for; // Check if the current loop is a for-loop
}

/**
 * Retrieves the depth of the current loop context.
 *
 * This function accesses the top loop context from the loops stack
 * and returns the depth at which the loop is currently operating.
 *
 * @param comp A pointer to the compiler instance.
 * @return The depth of the current loop context.
 */
int loop_depth(compiler_t *comp)
{
    // Return the depth of the loop at the top of the loops stack
    return ((loop_t *)top(comp->loops))->depth;
}

/**
 * Pushes a new function context onto the stack of contexts.
 * This function is used to manage the scope of functions and their
 * local variables.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 * @param name The name of the function to push onto the stack.
 */
void push_function(compiler_t *comp, char *name)
{
    if (!comp->is_lookUp)
    {
        // Store the current context depth and push a new context
        ((context_t *)top(comp->contexts))->depth = comp->current->depth;
        push(comp->contexts, create_context(true, list_create(sizeof(uint8_t)), name));

        // Update the current context to the new one
        comp->current = (context_t *)top(comp->contexts);
        comp->code = comp->current->code;
        comp->locals = comp->current->locals;
        comp->instrs = comp->current->instrs;
    }
}

/**
 * Pops the current function context from the stack of contexts.
 *
 * This function is used to manage the scope of functions and their
 * local variables. It is called when a function declaration is finished
 * and the function context should be popped from the stack.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 * @param params The number of parameters the function takes.
 */
void pop_function(compiler_t *comp, int params)
{
    if (!comp->is_lookUp)
    {
        char *name = comp->current->fun_name;
#ifndef __EMSCRIPTEN__
        printf("target <pifunction>: %s\n", name);
        dis(comp);
#endif

        int uv_size = list_size(comp->current->upvalues);
        list_t *upvalues = comp->current->upvalues;

        ObjCode *code = (ObjCode *)new_code(comp->code);
        int c_index = store_const(comp, NEW_OBJ(code));

        context_t *context = (context_t *)pop(comp->contexts);

        comp->current = (context_t *)top(comp->contexts);
        comp->code = comp->current->code;
        comp->locals = comp->current->locals;
        comp->instrs = comp->current->instrs;

        free(context);

        int n_index = store_const(comp, NEW_OBJ(new_pistring(name)));

        emit_16u(comp, OP_LOAD_CONST, name, n_index);

        char code_descr[32];
        snprintf(code_descr, sizeof(code_descr), "<code: 0x%04X>", code->hash);

        emit_16u(comp, OP_LOAD_CONST, code_descr, c_index);

        for (int i = 0; i < uv_size; i++)
        {
            upvalue_t *upvalue = (upvalue_t *)list_getAt(upvalues, i);
            int index = store_const(comp, NEW_NUM(upvalue->index));
            emit_16u(comp, OP_LOAD_CONST, itos(upvalue->index), index);
            index = store_const(comp, NEW_BOOL(upvalue->is_local));
            emit_16u(comp, OP_LOAD_CONST, upvalue->is_local ? "true" : "false", index);
        }

        if (uv_size > 0)
            emit_16u(comp, OP_PUSH_CLOSURE, name, (params << 8) | uv_size);
        else
            emit_8u(comp, OP_PUSH_FUNCTION, name, (byte)params);
    }
}
/**
 * Stores a value in the list of constants of the compiler.
 * If the value already exists in the list, its index is returned.
 * Otherwise, the value is added to the list and its index is returned.
 * @param comp A pointer to the compiler instance containing the list of constants.
 * @param value The value to store in the list of constants.
 * @return The index of the value in the list of constants.
 */
int store_const(compiler_t *comp, Value value)
{
    Value _value;
    // Iterate through the list of constants to see if the value already exists
    for (int i = 0; i < comp->constants->size; i++)
    {
        _value = *(Value *)list_getAt(comp->constants, i);
        // Check if the value already exists in the list
        if (equals(_value, value))
            return i;
    }
    // Add the value to the list if it doesn't already exist
    list_add(comp->constants, &value);
    // Return the index of the value in the list
    return comp->constants->size - 1;
}

/**
 * Adds a bytecode instruction to the code list of the compiler.
 * The instruction may have zero or more operands. The bytecode
 * instruction is added as a single byte, and each operand is added
 * as a single byte as well. The position of the last bytecode element
 * added is returned as the result.
 *
 * @param comp A pointer to the compiler instance containing the code and
 *             instructions lists.
 * @param opcode The opcode of the instruction to be added.
 * @param descr A description of the instruction to be added to the instruction
 *              list.
 * @param num_operands The number of operands of the instruction to be added.
 * @param ... The number of operands of the instruction to be added.
 * @return The index of the last bytecode element added, or -1 if an error occurs.
 */
static int _emit(compiler_t *comp, OpCode opcode, char *descr, int num_operands, int line, int column, ...)
{
    if (comp->code == NULL || comp->instrs == NULL || comp->is_lookUp)
        return -1;

    int size = list_size(comp->code);

    // Emit opcode
    uint8_t _opcode = (uint8_t)opcode;
    list_add(comp->code, &_opcode);

    // Emit operands
    uint8_t *operands = malloc(sizeof(uint8_t) * num_operands);
    if (!operands)
        return -1;

    va_list args;
    va_start(args, column); // column is now the last named param before variadic args
    for (int i = 0; i < num_operands; i++)
    {
        uint8_t operand = (uint8_t)va_arg(args, int);
        list_add(comp->code, &operand);
        operands[i] = operand;
    }
    va_end(args);

    // Create instruction info with line/column/description
    instr_t *info = malloc(sizeof(instr_t));
    info->descr = strdup(descr);
    info->line = line;
    info->column = column;
    info->offset = size;

    list_add(comp->instrs, info);
    return comp->code->size - 1;
}

// static int _emit(compiler_t *comp, OpCode opcode, char *descr, int num_operands, ...)
// {
//     if (comp->code == NULL || comp->instrs == NULL || comp->is_lookUp)
//         return -1; // Error handling: invalid code or instrs list

//     // Add the opcode as a byte
//     uint8_t _opcode = (uint8_t)opcode;
//     list_add(comp->code, &_opcode);

//     // Prepare operands for both the bytecode and the instruction description
//     uint8_t *operands = (uint8_t *)malloc(sizeof(uint8_t) * num_operands);
//     if (operands == NULL)
//         return -1; // Error handling: failed to allocate memory

//     // Initialize the variable argument list
//     va_list args;
//     va_start(args, num_operands);

//     // Iterate over each operand and add it to both the code list and the operands array
//     for (int i = 0; i < num_operands; i++)
//     {
//         uint8_t operand = (uint8_t)va_arg(args, int); // Read next operand
//         list_add(comp->code, &operand);               // Add to bytecode
//         operands[i] = operand;                        // Store in operands array
//     }

//     // Clean up the variable argument list
//     va_end(args);

//     list_add(comp->instrs, new_string(descr));
//     // Return the index of the last bytecode element added
//     return comp->code->size - 1;
// }

/**
 * Emits a single opcode with no operands to the bytecode.
 * @param comp A pointer to the compiler instance containing the code list.
 * @param opcode The opcode to be added to the code list.
 * @return The index of the last bytecode element added, or -1 if an error occurs.
 */
int emit(compiler_t *comp, OpCode opcode)
{
    return _emit(comp, opcode, "", 0, comp->current_line, comp->current_col);
}

/**
 * Emits a single opcode with one operand to the bytecode.
 * The operand is a single byte in the bytecode.
 * @param comp A pointer to the compiler instance containing the code list.
 * @param opcode The opcode to be added to the code list.
 * @param descr A description of the instruction to be added to the instruction list.
 * @param operand The single operand of the instruction to be added to the code list.
 * @return The index of the last bytecode element added, or -1 if an error occurs.
 */
int emit_8u(compiler_t *comp, OpCode opcode, char *descr, int operand)
{
    return _emit(comp, opcode, descr, 1, comp->current_line, comp->current_col, operand);
}

/**
 * Emits a single opcode with one operand to the bytecode.
 * The operand is a 16-bit short in the bytecode.
 * @param comp A pointer to the compiler instance containing the code list.
 * @param opcode The opcode to be added to the code list.
 * @param descr A description of the instruction to be added to the instruction list.
 * @param operand The 16-bit short operand of the instruction to be added to the code list.
 * @return The index of the last bytecode element added, or -1 if an error occurs.
 */
int emit_16u(compiler_t *comp, OpCode opcode, char *descr, int operand)
{
    byte op1 = (byte)((operand >> 8) & 0xff);
    byte op2 = (byte)(operand & 0xff);
    return _emit(comp, opcode, descr, 2, comp->current_line, comp->current_col, op1, op2);
}

/**
 * Emits the OP_POP_N instruction to pop a certain number of local variables
 * from the stack. If the size is 1, it emits the OP_POP instruction instead.
 * @param comp A pointer to the compiler instance containing the code list.
 * @param depth The depth of the local variables to pop.
 * @return The number of local variables popped from the stack.
 */
int emit_pop(compiler_t *comp, int depth)
{
    int size = get_localSize(comp, depth);
    if (size > 1)
        emit_8u(comp, OP_POP_N, "", (uint8_t)size); // Pop multiple locals
    else if (size == 1)
        emit(comp, OP_POP); // Pop a single local
    return size;
}

/**
 * Emits a jump instruction to the bytecode.
 * @param comp A pointer to the compiler instance containing the code list.
 * @param address The address to jump to in the bytecode.
 * @return The index of the last bytecode element added, or -1 if an error occurs.
 */
int emit_jump(compiler_t *comp, int address)
{
    // Emit a jump instruction with the given address
    emit_16u(comp, OP_JUMP, "", address);
    // Return the index of the last bytecode element added
    return comp->code->size - 1;
}

void patch_jump(compiler_t *comp, int address)
{
    if (!comp->is_lookUp)
    {

        int offset = comp->code->size - (address - 2);

        uint8_t *code = (uint8_t *)comp->code->data;
        code[address - 1] = (offset >> 8) & 0xff;
        code[address] = offset & 0xff;
    }
}

// void patch_jump(compiler_t *comp, int address)
// {
//     if (!comp->is_lookUp)
//     {
//         // Get the current bytecode position (where to jump)
//         int jump = comp->code->size; // This function should return the current position

//         uint8_t *code = (uint8_t *)comp->code->data;

//         // Calculate the high and low bytes of the jump offset
//         uint8_t high = (jump >> 8) & 0xff;
//         uint8_t low = jump & 0xff;

//         // Patch the bytecode at the given address (address - 1 for high byte)
//         code[address - 1] = high; // Update the high byte of the jump address
//         code[address] = low;      // Update the low byte of the jump address
//     }
// }
int code_size(compiler_t *comp)
{
    return comp->code->size;
}
void dis(compiler_t *comp)
{
    int pc = 0;
    int line = 0, i = 0;
    list_t *instrs = comp->instrs;
    uint8_t *code = (uint8_t *)comp->code->data;
    int length = comp->code->size;

    while (pc < length)
    {
        uint8_t op = code[pc++];
        OpCode opcode = (OpCode)op;
        instr_t *instr = list_getAt(instrs, i);
        char *descr = instr->descr;
        switch (opcode)
        {
        case OP_STORE_GLOBAL:
        case OP_STORE_LOCAL:
        case OP_LOAD_GLOBAL:
        case OP_LOAD_LOCAL:
        case OP_LOAD_UPVALUE:
        case OP_STORE_UPVALUE:
        case OP_BINARY:
        case OP_COMPARE:
        case OP_UNARY:
        case OP_POP_N:
        case OP_CALL_FUNCTION:
        case OP_PUSH_FUNCTION:
            // Print: line number, opcode name, and one operand:
            printf("\033[38;2;107;107;107m%-3d\033[0m: " // Dark Gray for line numbers
                   "\033[38;2;139;0;0m%-15s\033[0m "     // Dark Red for opcode names
                   "\033[38;2;184;134;11m%-5d\033[0m",   // Dark Yellow for numeric operand
                   line++, op_names[opcode], code[pc]);
            line++;
            pc++;
            break;

        case OP_JUMP_IF_FALSE:
        case OP_JUMP:
        case OP_LOOP:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]);
            int target = pc + offset - 1;

            const char *fmt =
                offset < 0
                    ? "\033[38;2;107;107;107m%-3d\033[0m: \033[38;2;139;0;0m%-14s\033[0m \033[38;2;184;134;11m%-6d\033[0m \033[38;2;34;139;34m[<< %-3d]\033[0m\n"
                    : "\033[38;2;107;107;107m%-3d\033[0m: \033[38;2;139;0;0m%-14s\033[0m \033[38;2;184;134;11m%-6d\033[0m \033[38;2;34;139;34m[>> %-3d]\033[0m\n";

            printf(fmt, line++, op_names[opcode], offset, target);
            line += 2;
            pc += 2;
            i++;      // to sync with instrs list
            continue; // skip the shared description logic since we handled it here
        }

        case OP_LOAD_CONST:
        case OP_PUSH_LIST:
        case OP_PUSH_MAP:
        {
            // Print: line number, opcode name, and a short operand:
            printf("\033[38;2;107;107;107m%-3d\033[0m: "
                   "\033[38;2;139;0;0m%-15s\033[0m "
                   "\033[38;2;184;134;11m%-5d\033[0m",
                   line++, op_names[opcode], read_short(comp, pc));
            line += 2;
            pc += 2;
            break;
        }

        case OP_PUSH_CLOSURE:
        {
            // Print: line number, opcode name, and a short operand:
            printf("\033[38;2;107;107;107m%-3d\033[0m: "
                   "\033[38;2;139;0;0m%-15s\033[0m "
                   "\033[38;2;184;134;11m%d %3d\033[0m",
                   line++, op_names[opcode], code[pc], code[pc + 1]);
            line += 2;
            pc += 2;
            break;
        }

        default:
            // Default: line number and opcode name only:
            printf("\033[38;2;107;107;107m%-3d\033[0m: "
                   "\033[38;2;139;0;0m%-15s\033[0m",
                   line++, op_names[opcode]);
            break;
        }

        if (strcmp(descr, "") != 0)
        {
            if (strlen(descr) > 20)
            {
                char *_descr = malloc(20);
                strncpy(_descr, descr, 20);
                _descr[19] = '\0';
                printf(" \033[38;2;34;139;34m[%s...]\033[0m\n", _descr); // Dark Green for descriptors
            }
            else
                printf(" \033[38;2;34;139;34m[%s]\033[0m\n", descr);
        }
        else
            printf("\n");
        i++;
    }
    printf("\n");
}

void free_compiler(compiler_t *comp)
{

    list_free(comp->code);
    list_free(comp->constants);
    list_free(comp->names);
    stack_free(comp->locals);
    stack_free(comp->contexts);
    stack_free(comp->loops);
    // list_free(comp->instrs);
    stack_free(comp->objects);

    free(comp);
}