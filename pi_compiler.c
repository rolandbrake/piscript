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

#include "builtin/pi_builtin.h"

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
    context->instrs = list_create(sizeof(instr_t));

    context->is_function = is_function;
    context->depth = 0;
    context->code = code;

    if (fun_name == NULL && is_function)
    {
        context->fun_name = malloc(32); // Adjust size as needed
        sprintf(context->fun_name, "<LAMBDA: %d>", f_count);
        f_count++;
    }
    else
        context->fun_name = fun_name;

    return context;
}

/**
 * Frees the contents of an instr_t struct.
 * This includes dynamically allocated strings and operand arrays.
 * @param instr The instruction to free.
 */
static void free_instr(instr_t *instr)
{
    free(instr->descr);
    free(instr->fun_name);
    free(instr->operands);
    free(instr);
}

/**
 * Frees the contents of a context_t struct.
 * This includes its upvalues, locals, and instruction metadata.
 * It also handles nested freeing for instr_t.
 * @param context The context to free.
 */
static void free_context(context_t *context)
{
    // Free upvalues
    list_free(context->upvalues);

    // Free locals (local_t structs contain strdup'd names)
    while (!is_empty(context->locals))
    {
        local_t *local = (local_t *)pop(context->locals);
        free(local->name);
        free(local);
    }
    stack_free(context->locals);

    // Free instrs list and their contents
    if (context->instrs)
    {
        while (!list_isEmpty(context->instrs))
        {
            instr_t *instr = (instr_t *)list_pop(context->instrs);
            free_instr(instr);
        }
        list_free(context->instrs);
    }

    free(context->fun_name);
    free(context);
}

/**
 * Frees the contents of a loop_t struct.
 * This includes its breaks stack.
 * @param loop The loop to free.
 */
static void free_loop(loop_t *loop)
{
    stack_free(loop->breaks); // Assuming stack_free also frees elements if they are pointers. Need to check stack_t.
    free(loop);
}

static void free_loop(loop_t *loop);




/**
 * Retrieves the current active context from the stack of contexts.
 *
 * This function is a convenience wrapper to access the top element of the
 * contexts stack. It is used to access the current active compilation context
 * in the compiler.
 *
 * @param comp[in] A pointer to the compiler instance containing the stack of contexts.
 *
 * @return A pointer to the current active context.
 */
static context_t *current_context(compiler_t *comp)
{
    return (context_t *)top(comp->contexts);
}

/**
 * Creates a new upvalue structure with the given index and is_local flag.
 *
 * This function is used to create a new upvalue structure when a variable is
 * captured by a closure. The upvalue structure is used to store the index and
 * is_local flag of the captured variable.
 *
 * @param index[in] The index of the captured variable.
 * @param is_local[in] A flag indicating if the captured variable is a local
 *                     variable.
 *
 * @return A pointer to the new upvalue structure.
 */
static upvalue_t *create_upvalue(int index, bool is_local)
{
    upvalue_t *upvalue = malloc(sizeof(upvalue_t));

    upvalue->index = index;
    upvalue->is_local = is_local;
    return upvalue;
}

/**
 * Initializes a new compiler instance.
 *
 * This function allocates memory for a compiler structure and initializes its
 * members. It creates empty lists for the code, constants, names, and
 * instruction metadata. It also initializes the stack of local variables, the
 * stack of contexts, the stack of objects, and the stack of loops.
 *
 * @return A pointer to the newly initialized compiler instance.
 */
compiler_t *init_compiler()
{

    // Allocate memory for the compiler structure
    compiler_t *comp = (compiler_t *)malloc(sizeof(compiler_t));

    // Initialize list_t members
    comp->code = list_create(sizeof(uint8_t));
    comp->constants = list_create(sizeof(Value));

    // Initialize the constants list with NaN, Infinity, true, and false
    list_add(comp->constants, &NEW_NUM(NAN));
    list_add(comp->constants, &NEW_NUM(INFINITY));

    list_add(comp->constants, &NEW_BOOL(true));
    list_add(comp->constants, &NEW_BOOL(false));

    // names for storing the names of the global variables
    comp->names = list_create(sizeof(String));
    comp->builtin_names = list_create(sizeof(String));

    // Register built-in constant names
    for (int i = 0; i < BUILTIN_CONST_COUNT; i++)
        list_add(comp->builtin_names, new_string(builtin_constants[i].name));

    // Register built-in function names
    for (int i = 0; i < BUILTIN_FUNC_COUNT; i++)
        list_add(comp->builtin_names, new_string(builtin_functions[i].name));

    // Initialize stack_t members
    comp->locals = stack_create(sizeof(local_t));
    comp->contexts = stack_create(sizeof(context_t));
    comp->loops = stack_create(sizeof(loop_t));
    comp->objects = stack_create(sizeof(String));
    comp->name = "";

    // Initialize the current <global> context
    comp->current = create_context(false, comp->code, NULL);

    // Initialize instruction table with global scope
    comp->instrs = ht_create(sizeof(list_t));

    comp->is_lookUp = false;
    comp->is_upvalue = false;
    comp->is_repl = false;

    push(comp->contexts, comp->current);

    return comp;
}

/**
 * Reads a 16-bit short from the bytecode at the specified index.
 *
 * This function retrieves two consecutive bytes from the compiler's bytecode,
 * combines them, and returns the resulting 16-bit short integer.
 *
 * @param comp[in] A pointer to the compiler instance containing the bytecode.
 * @param index[in] The index in the bytecode from where to start reading.
 *
 * @return The 16-bit short integer constructed from the bytecode.
 */
static int read_short(compiler_t *comp, int index)
{
    uint8_t *code = (uint8_t *)comp->code->data; // Access the bytecode from the compiler's code list
    int high = code[index] & 0xFF;               // Get the high byte and mask it to 8 bits
    int low = code[index + 1] & 0xFF;            // Get the low byte and mask it to 8 bits

    return (high << 8) | low; // Combine high and low bytes into a 16-bit short
}

/**
 * @brief Adds a byte to the compiler's code list.
 *
 * This function ensures the code list has enough capacity to accommodate
 * the new byte. If the list is full, it reallocates more space.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param _byte[in] The byte to be added to the code list.
 */
void add_code(compiler_t *comp, byte _byte)
{
    // Ensure the compiler and its code list are valid
    if (!comp || !comp->code)
    {
        fprintf(stderr, "Invalid compiler or code list\n");
        exit(EXIT_FAILURE);
    }

    int size = comp->code->size;
    int cap = comp->code->capacity;

    // Check if the code list needs expansion
    if (size == cap)
    {
        cap = cap == 0 ? 8 : cap * 2;
        comp->code->data = realloc(comp->code->data, cap * sizeof(byte));
        if (comp->code->data == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        comp->code->capacity = cap;
    }

    // Add the byte to the code list
    ((byte *)comp->code->data)[size++] = _byte;
    comp->code->size = size;
}

/**
 * @brief Checks if the current scope is a local scope.
 *
 * This function determines whether the current compilation context
 * is within a local scope. A local scope is identified by having
 * a scope depth greater than zero or by being within a function context.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 * @return true if the current scope is local, false otherwise.
 */
bool is_localScope(compiler_t *comp)
{
    // A scope is considered local if its depth is greater than zero
    // or if it is within a function context.
    return comp->current->depth > 0 || comp->current->is_function;
}

/**
 * Pushes a new object onto the stack of objects being allocated.
 *
 * This function is used to manage the allocation of objects
 * within the compiler's current context. It ensures that the
 * new object is added to the stack unless a lookup operation
 * is being performed.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 */
void push_object(compiler_t *comp)
{
    if (!comp->is_lookUp)
        // Push a new object onto the stack using the current variable name
        push(comp->objects, new_string(comp->name));
}

/**
 * Pops the current object from the stack of objects being allocated.
 *
 * This function is used to manage the allocation of objects
 * within the compiler's current context. It ensures that the
 * current object is removed from the stack unless a lookup operation
 * is being performed.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 */
void pop_object(compiler_t *comp)
{
    if (!comp->is_lookUp)
        // Pop the current object from the stack of objects being allocated
        pop(comp->objects);
}

/**
 * Checks if the compiler is currently compiling an object.
 *
 * @param comp[in] A pointer to the compiler instance containing the current context.
 * @return true if the compiler is compiling an object, false otherwise.
 */
bool is_object(compiler_t *comp)
{
    return !is_empty(comp->objects);
}

/**
 * Determines if the current context is a constructor function.
 *
 * This function checks whether the compiler is currently
 * compiling a function named "constructor" within an object context.
 *
 * @param comp A pointer to the compiler instance containing the current context.
 * @return true if the current context is a constructor function, false otherwise.
 */
bool is_constructor(compiler_t *comp)
{
    // Check if the current context is an object and a function
    if (is_object(comp) && comp->current->is_function)
        // Compare the current function name with "constructor"
        return strcmp(comp->current->fun_name, "constructor") == 0;

    return false;
}
/**
 * Checks if the compiler is currently performing a lookup operation.
 *
 * This function returns the status of the is_lookUp flag in the compiler
 * instance, indicating whether a lookup operation is in progress.
 *
 * @param comp A pointer to the compiler instance containing the lookup flag.
 * @return true if a lookup operation is in progress, false otherwise.
 */
bool is_lookUp(compiler_t *comp)
{
    return comp->is_lookUp;
}

/**
 * @brief Sets the lookup flag in the compiler and returns the previous value.
 *
 * This function is used to control whether the compiler is currently
 * performing a lookup operation. It is used to prevent the compiler
 * from capturing variables that are not intended to be captured.
 *
 * @param comp A pointer to the compiler instance containing the lookup flag.
 * @param value The new value for the lookup flag.
 * @return The previous value of the lookup flag.
 */
bool look_up(compiler_t *comp, bool value)
{
    // Store the current value of the lookup flag
    bool look_up = comp->is_lookUp;

    // Update the lookup flag with the new value
    comp->is_lookUp = value;

    // Return the previous value of the lookup flag
    return look_up;
}

/**
 * Prints the current local variables in the compiler
 *
 * This function is used for debugging purposes and prints the current
 * local variables in the compiler to the console. It prints the name,
 * depth, and whether the variable is captured as an upvalue.
 *
 * @param comp A pointer to the compiler instance containing the locals stack.
 */
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

/**
 * @brief Adds a new local variable to the current scope.
 *
 * This function checks if the variable with the given name already exists
 * in the current scope. If it does, an error is reported. Otherwise, it
 * allocates a new local variable, sets its properties, and pushes it onto
 * the stack of local variables.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param name[in] The name of the local variable to add.
 */
void add_local(compiler_t *comp, char *name)
{
    stack_t *locals = comp->current->locals;

    // Check for name conflict ONLY in current block
    for (int i = stack_size(locals) - 1; i >= 0; i--)
    {
        local_t *local = (local_t *)stack_getAt(locals, i);

        if (local->depth < comp->current->depth)
            break; // Stop checking once we’re outside the current block

        if (strcmp(local->name, name) == 0)
            p_errorf(comp->current_line, comp->current_col,
                     "Name already declared in this scope: [%s]", name);
    }

    // Allocate and initialize a new local variable
    local_t *local = malloc(sizeof(local_t));
    local->name = strdup(name); // Allocate and copy name string
    local->depth = comp->current->depth;
    local->is_captured = false;

    // Push the new local variable onto the stack
    push(locals, local);
}

/**
 * @brief Retrieves the index of a local or upvalue variable by name.
 *
 * This function searches for a variable with the given name in the
 * current context. It first attempts to resolve the variable as a
 * local variable. If it is not found, it checks for the variable as
 * an upvalue, updating the compiler's state to indicate the result.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param name[in] The name of the variable to retrieve.
 * @return The index of the variable if found, otherwise -1.
 */
int get_local(compiler_t *comp, char *name)
{
    // Initialize the variable index to -1
    int index = -1;
    // Determine the current depth of the context stack
    int depth = stack_size(comp->contexts) - 1;
    // Reset the upvalue flag
    comp->is_upvalue = false;

    // Attempt to resolve the variable as a local variable
    index = resolve_local(comp, depth, name);
    if (index != -1)
        return index;
    else
    {
        // Attempt to resolve the variable as an upvalue
        index = resolve_upvalue(comp, depth, name);
        // Update the upvalue flag if found
        comp->is_upvalue = true;
    }
    // Return the resolved index or -1 if not found
    return index;
}

/**
 * @brief Retrieves the number of local variables at a given depth.
 *
 * This function calculates the number of local variables declared
 * at or above a given depth. It iterates through the stack of local
 * variables, counting the number of variables until it finds a
 * variable with a depth less than the given depth.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param depth[in] The depth at which to retrieve the local variable count.
 * @return The number of local variables at the given depth.
 */
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

/**
 * @brief Resolves a local variable in the current context.
 *
 * This function searches for a local variable with the given name
 * in the current context. It iterates through the stack of local
 * variables and checks each variable for a match with the given
 * name.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param depth[in] The depth of the context to search.
 * @param name[in] The name of the variable to search for.
 * @return The index of the variable if found, otherwise -1.
 */
int resolve_local(compiler_t *comp, int depth, char *name)
{
    int index = -1;
    local_t *local;
    context_t *context = stack_getAt(comp->contexts, depth);
    // Iterate through the stack of local variables in reverse order
    for (int i = stack_size(context->locals) - 1; i >= 0; i--)
    {
        local = (local_t *)stack_getAt(context->locals, i);
        // Check if the current local variable matches the given name
        if (strcmp(local->name, name) == 0)
        {
            index = i;
            break;
        }
    }
    // Return the resolved index or -1 if not found
    return index;
}

/**
 * @brief Resolves an upvalue in the current context.
 *
 * This function attempts to resolve a variable as an upvalue by first
 * checking the enclosing local scope and then any existing upvalues.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @param depth[in] The depth of the context to search.
 * @param name[in] The name of the variable to search for.
 * @return The index of the upvalue if found, otherwise -1.
 */
int resolve_upvalue(compiler_t *comp, int depth, char *name)
{
    // Base case: if the depth is 0, stop searching and return -1
    if (depth == 0)
        return -1;

    // Attempt to resolve the variable as a local variable in the enclosing scope
    int index = resolve_local(comp, depth - 1, name);
    if (index != -1)
        // Add the local variable as an upvalue and return its index
        return add_upvalue(comp, depth, index, true);

    // Attempt to resolve the variable as an upvalue in the enclosing scope
    int upvalue = resolve_upvalue(comp, depth - 1, name);
    if (upvalue != -1)
        // Add the existing upvalue and return its index
        return add_upvalue(comp, depth, upvalue, false);

    // If the variable cannot be resolved as an upvalue, return -1
    return -1;
}

/**
 * Adds an upvalue to the given context.
 *
 * This function adds an upvalue to the given context if it does not already
 * exist. It returns the index of the upvalue in the context's upvalue list.
 *
 * @param comp[in] The compiler instance.
 * @param depth[in] The depth of the context to modify.
 * @param index[in] The index of the variable to add as an upvalue.
 * @param is_local[in] A flag indicating if the variable is a local variable or
 *                     an upvalue from an outer scope.
 * @return The index of the upvalue in the context's upvalue list.
 */
int add_upvalue(compiler_t *comp, int depth, int index, bool is_local)
{
    // Get the context at the given depth
    context_t *current = stack_getAt(comp->contexts, depth);

    // Check if the upvalue already exists in the context's upvalue list
    int size = list_size(current->upvalues);
    for (int i = 0; i < size; i++)
    {
        // Get the upvalue at the current index
        upvalue_t *_upvalue = (upvalue_t *)list_getAt(current->upvalues, i);

        // If the upvalue already exists, return its index
        if (_upvalue->index == index && _upvalue->is_local == is_local)
            return i;
    }

    // If the upvalue does not exist, create a new upvalue structure and add it to
    // the context's upvalue list
    upvalue_t *upvalue = malloc(sizeof(upvalue_t));
    upvalue->index = index;
    upvalue->is_local = is_local;
    list_add(current->upvalues, upvalue);

    // Return the index of the new upvalue
    return size - 1;
}

/**
 * Checks if the given name is a built-in constant or function.
 * @param comp The compiler instance.
 * @param name The name to check.
 * @return True if the name is a built-in constant or function, false otherwise.
 */
bool is_builtin(compiler_t *comp, const char *name)
{
    // Iterate through the list of built-in names stored in the compiler
    for (int i = 0; i < comp->builtin_names->size; i++)
    {
        // Get the current name from the list
        char *existing = string_get(comp->builtin_names, i);
        // Compare the current name with the target name
        if (strcmp(existing, name) == 0)
            return true; // Return true if a match is found
    }
    return false; // Return false if the name is not found in the list
}

/**
 * Adds a new variable to the current scope.
 * If the variable is local, it checks if the variable is already declared.
 * If the variable is global, it stores the variable in the global scope.
 * @param comp The compiler instance.
 * @param name The name of the variable to add.
 */
void add_variable(compiler_t *comp, char *name)
{
    int g_index = -1;
    // Check if the variable is local or global
    if (is_localScope(comp))
    {
        // Add the local variable to the current scope
        add_local(comp, name);
    }
    else
    {
        // Check if the global variable already exists
        g_index = name_index(comp, name);
        if (g_index != -1 || is_builtin(comp, name))
            // Error if the variable already exists
            p_errorf(comp->current_line, comp->current_col, "Name already exists [%s]", name);

        // Store the global variable
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
            // Store the variable in the local scope
            emit_8u(comp, comp->is_upvalue ? OP_STORE_UPVALUE : OP_STORE_LOCAL, name, index);
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

/**
 * Loads a variable from the current scope or global scope.
 *
 * This function attempts to load a variable by its name from the current
 * local scope first. If the variable is not found locally, it attempts
 * to load it from the global scope.
 *
 * @param comp A pointer to the compiler instance.
 * @param name The name of the variable to load.
 */
void load_variable(compiler_t *comp, char *name)
{
    // Attempt to find the variable in the local scope
    int index = get_local(comp, name);
    if (index != -1)
    {
        // Emit an instruction to load from the appropriate scope
        if (comp->is_upvalue)
            emit_8u(comp, OP_LOAD_UPVALUE, name, index);
        else
            emit_8u(comp, OP_LOAD_LOCAL, name, index);
    }
    else
    {
        // Attempt to find the variable in the global scope
        int g_index = name_index(comp, name);
        if (g_index == -1)
            // If not found, store the name in the global scope
            g_index = store_name(comp, name);
        // Emit an instruction to load from the global scope
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

/**
 * Stores a new name in the compiler's list of names.
 *
 * This function adds a new name to the list of names stored in the compiler.
 * If the name already exists in the list, it returns the existing index.
 * Otherwise, it adds the name and returns the new index.
 *
 * @param comp A pointer to the compiler instance containing the list of names.
 * @param name The name to store in the list.
 * @return The index of the name in the list.
 */
int store_name(compiler_t *comp, char *name)
{
    int index = name_index(comp, name);
    if (index != -1)
        return index; // Name already exists, return the index

    // Add the name to the list of names
    list_add(comp->names, new_string(name));

    // Return the new index
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
 * Checks if the compiler is currently inside a loop.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @return true if the compiler is inside a loop, false otherwise.
 */
bool in_loop(compiler_t *comp)
{
    return !is_empty(comp->loops); // Check if the loops stack is empty
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
        context_t *context = create_context(true, list_create(sizeof(uint8_t)), name);
        push(comp->contexts, context);

        // Initialize instruction list for this function
        list_t *instrs = list_create(sizeof(instr_t));
        ht_put(comp->instrs, strdup(context->fun_name), instrs);

        // Update the current context to the new one
        comp->current = (context_t *)top(comp->contexts);
        comp->code = comp->current->code;
        comp->locals = comp->current->locals;
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

        list_t *instrs = ht_get(comp->instrs, name);

        // Move all instructions from current context to the hash table
        for (int i = 0; i < comp->current->instrs->size; i++)
        {
            instr_t *instr = list_getAt(comp->current->instrs, i);
            list_add(instrs, instr);
        }

        ht_put(comp->instrs, name, comp->current->instrs);

        int uv_size = list_size(comp->current->upvalues);
        list_t *upvalues = comp->current->upvalues;

        ObjCode *code = (ObjCode *)new_code(comp->code);
        int c_index = store_const(comp, NEW_OBJ(code));

        context_t *context = (context_t *)pop(comp->contexts);

        comp->current = (context_t *)top(comp->contexts);
        comp->code = comp->current->code;
        comp->locals = comp->current->locals;

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
    instr_t *instr = malloc(sizeof(instr_t));
    instr->descr = strdup(descr);
    instr->line = line;
    instr->column = column;
    instr->offset = size;
    if (comp->current->fun_name != NULL)
        instr->fun_name = strdup(comp->current->fun_name);
    else
        instr->fun_name = NULL;

    instr->opcode = opcode;
    instr->num_operands = num_operands;
    instr->operands = operands;

    list_add(comp->current->instrs, instr);

    return comp->code->size - 1;
}

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

/**
 * @brief Patches the jump instruction at the given address with the correct offset.
 *
 * This function calculates the offset for a jump instruction and updates the bytecode
 * and instruction metadata accordingly.
 *
 * @param comp A pointer to the compiler instance containing the bytecode and metadata.
 * @param address The address of the jump instruction to be patched.
 */
void patch_jump(compiler_t *comp, int address)
{
    if (!comp->is_lookUp)
    {
        // Calculate the offset for the jump instruction
        int offset = comp->code->size - (address - 2);

        // Update the bytecode with the calculated offset
        uint8_t *code = (uint8_t *)comp->code->data;
        code[address - 1] = (offset >> 8) & 0xff;
        code[address] = offset & 0xff;

        // Update the instruction metadata with the new operands
        for (int i = list_size(comp->current->instrs) - 1; i >= 0; i--)
        {
            instr_t *instr = list_getAt(comp->current->instrs, i);
            if (instr->offset == address - 2)
            {
                instr->operands[0] = (offset >> 8) & 0xff;
                instr->operands[1] = offset & 0xff;
                break;
            }
        }
    }
}

/**
 * @brief Returns the size of the bytecode list in the compiler instance.
 *
 * This function is useful for determining the size of the bytecode
 * that has been generated by the compiler.
 *
 * @param comp[in] A pointer to the compiler instance.
 * @return The size of the bytecode list.
 */
int code_size(compiler_t *comp)
{
    return comp->code->size;
}

/**
 * Disassembles the compiled bytecode for debugging purposes.
 *
 * This function outputs the disassembled instructions of the compiled
 * bytecode, differentiating between global and function-specific instructions.
 * It provides a human-readable format with color-coded elements for better
 * visibility.
 *
 * @param comp The compiler instance containing the bytecode and metadata.
 */

void dis(compiler_t *comp)
{

    printf("disassembling...\n");

    // Ensure global scope instructions are in the hash table
    if (stack_size(comp->contexts) > 0)
    {
        context_t *global_ctx = (context_t *)stack_getAt(comp->contexts, 0);
        ht_put(comp->instrs, "<global>", global_ctx->instrs);
    }

    // Get all function names in order
    char **scope_names = ht_keys(comp->instrs);

    int size = ht_length(comp->instrs);

    for (int i = 0; i < size; i++)
    {
        char *scope_name = scope_names[i];
        list_t *instrs = ht_get(comp->instrs, scope_name);

        printf("\n\033[1;36m== Disassembly of %s ==\033[0m\n\n",
               strcmp(scope_name, "<global>") == 0 ? "global scope" : scope_name);

        if (!instrs)
            continue;

        int line = 0, pc = 0;
        for (int j = 0; j < instrs->size; j++)
        {
            instr_t *instr = (instr_t *)list_getAt(instrs, j);
            OpCode opcode = instr->opcode;
            uint8_t *operands = instr->operands;
            char *descr = instr->descr;

            char line_buf[256] = {0};

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
                snprintf(line_buf, sizeof(line_buf),
                         "\033[38;2;107;107;107m%-4d\033[0m: "
                         "\033[38;2;139;0;0m%-15s\033[0m "
                         "\033[38;2;184;134;11m%-5d\033[0m",
                         line++, op_names[opcode], operands[0]);
                line++;
                pc++;
                break;

            case OP_JUMP_IF_FALSE:
            case OP_JUMP:
            case OP_LOOP:
            {
                int offset = (int16_t)((operands[0] << 8) | operands[1]);
                int target = instr->offset + offset;

                snprintf(line_buf, sizeof(line_buf),
                         offset < 0
                             ? "\033[38;2;107;107;107m%-4d\033[0m: \033[38;2;139;0;0m%-14s\033[0m "
                               "\033[38;2;184;134;11m%-6d\033[0m \033[38;2;34;139;34m[<< %-3d]\033[0m\n"
                             : "\033[38;2;107;107;107m%-4d\033[0m: \033[38;2;139;0;0m%-14s\033[0m "
                               "\033[38;2;184;134;11m%-6d\033[0m \033[38;2;34;139;34m[>> %-3d]\033[0m\n",
                         line++, op_names[opcode], offset, target);
                line += 2;
                pc += 2;

                printf("%s", line_buf);
                continue;
            }

            case OP_LOAD_CONST:
            case OP_PUSH_LIST:
            case OP_PUSH_MAP:
                snprintf(line_buf, sizeof(line_buf),
                         "\033[38;2;107;107;107m%-4d\033[0m: "
                         "\033[38;2;139;0;0m%-15s\033[0m "
                         "\033[38;2;184;134;11m%-5d\033[0m",
                         line++, op_names[opcode], (int16_t)((operands[0] << 8) | operands[1]));
                line += 2;
                pc += 2;
                break;

            case OP_PUSH_CLOSURE:
                snprintf(line_buf, sizeof(line_buf),
                         "\033[38;2;107;107;107m%-4d\033[0m: "
                         "\033[38;2;139;0;0m%-15s\033[0m "
                         "\033[38;2;184;134;11m%d %3d\033[0m",
                         line++, op_names[opcode], operands[0], operands[1]);
                line += 2;
                pc += 2;
                break;

            default:
                snprintf(line_buf, sizeof(line_buf),
                         "\033[38;2;107;107;107m%-4d\033[0m: "
                         "\033[38;2;139;0;0m%-15s\033[0m",
                         line++, op_names[opcode]);
                break;
            }

            // Print description
            if (descr && strcmp(descr, "") != 0)
            {
                if (strlen(descr) > 20)
                {
                    char short_descr[21];
                    strncpy(short_descr, descr, 20);
                    short_descr[20] = '\0';
                    strcat(line_buf, " \033[38;2;34;139;34m[");
                    strcat(line_buf, short_descr);
                    strcat(line_buf, "...]\033[0m\n");
                }
                else
                {
                    strcat(line_buf, " \033[38;2;34;139;34m[");
                    strcat(line_buf, descr);
                    strcat(line_buf, "]\033[0m\n");
                }
            }
            else
                strcat(line_buf, "\n");

            printf("%s", line_buf);
        }
    }
}

/**
 * Reports a parsing error with a specified message, line, and column.
 *
 * This function outputs an error message to the standard error stream,
 * indicating the location (line and column) and description of a parsing error.
 * If a custom error handler is set, it will be called instead of exiting.
 *
 * @param message The error message to be displayed.
 * @param line The line number where the error occurred.
 * @param column The column number where the error occurred.
 */
void p_error(const char *message, int line, int column)
{
    if (global_errorHandler)
        global_errorHandler(message, line, column);
    else
    {
        // Print the error message with the specified line and column
        fprintf(stderr, "[Parsing Error] at line %d, column %d: %s\n",
                line, column, message);
        exit(EXIT_FAILURE);
    }
}

/**
 * Reports a parsing error with a specified message, line, and column,
 * using a variable-argument list for formatting the message.
 *
 * This function outputs an error message to the standard error stream,
 * indicating the location (line and column) and description of a parsing error.
 * If a custom error handler is set, it will be called instead of exiting.
 *
 * @param line The line number where the error occurred.
 * @param column The column number where the error occurred.
 * @param format The format string for the error message.
 * @param ... The variable arguments to be formatted into the message.
 */
void p_errorf(int line, int column, const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (global_errorHandler)
    {
        global_errorHandler(buffer, line, column);
    }
    else
    {
        // Flush stdout before printing to stderr
        fflush(stdout);

        // Print the error message with the specified line and column
        fprintf(stderr, "\n\033[1;31m[PARSE ERROR] at line %d, column %d:\033[0m %s", line, column, buffer);
        fprintf(stderr, "\n");

        exit(EXIT_FAILURE);
    }
}

/**
 * Frees the memory allocated for a compiler instance.
 *
 * @param comp The compiler instance to be deallocated.
 */
void free_compiler(compiler_t *comp)
{
    // Free the bytecode instructions list
    list_free(comp->code);

    // Free the constant values list
    list_free(comp->constants); // Values are copied, not pointers to new allocations

    // Free the variable names list and their contents
    list_free(comp->names);

    // Free the built-in names list and their contents
    list_free(comp->builtin_names);

    // Free the contexts stack and their contents
    while (!is_empty(comp->contexts))
    {
        context_t *context = (context_t *)pop(comp->contexts);
        free_context(context);
    }
    stack_free(comp->contexts);

    // Free the loops stack and their contents
    while (!is_empty(comp->loops))
    {
        loop_t *loop = (loop_t *)pop(comp->loops);
        free_loop(loop);
    }
    stack_free(comp->loops);

    // Free the objects stack (it contains String* but these are transient and not part of output bytecode)
    stack_free(comp->objects);

    // Free the instruction table (ht_create(sizeof(list_t)) - list_t* values)
    char **keys = ht_keys(comp->instrs);
    for (int i = 0; i < ht_length(comp->instrs); i++)
    {
        char *key = keys[i];
        list_t *instr_list = (list_t *)ht_get(comp->instrs, key);
        while (!list_isEmpty(instr_list))
        {
            instr_t *instr = (instr_t *)list_pop(instr_list);
            free_instr(instr);
        }
        list_free(instr_list);
        free(key); // Free the key string from the hash table
    }
    ht_free(comp->instrs);

    // Free the compiler structure itself
    free(comp);
}

/**
 * Resets a compiler instance to its initial state, allowing it to be reused.
 *
 * This function performs a deep cleanup of all dynamically allocated data
 * within the compiler's internal structures and then reinitializes them
 * to their default empty states, ready for a new compilation task.
 * The compiler_t struct itself is not freed, only its contents.
 *
 * @param comp The compiler instance to reset.
 */
void reset_compiler(compiler_t *comp)
{
    // 1. Deep free existing resources
    list_free(comp->code);
    list_free(comp->names);

    while (!is_empty(comp->contexts))
    {
        context_t *context = (context_t *)pop(comp->contexts);
        free_context(context);
    }
    stack_free(comp->contexts);

    while (!is_empty(comp->loops))
    {
        loop_t *loop = (loop_t *)pop(comp->loops);
        free_loop(loop);
    }
    stack_free(comp->loops);

    stack_free(comp->objects);

    char **keys = ht_keys(comp->instrs);
    for (int i = 0; i < ht_length(comp->instrs); i++)
    {
        char *key = keys[i];
        list_t *instr_list = (list_t *)ht_get(comp->instrs, key);
        while (!list_isEmpty(instr_list))
        {
            instr_t *instr = (instr_t *)list_pop(instr_list);
            free_instr(instr);
        }
        list_free(instr_list);
        free(key);
    }
    ht_free(comp->instrs);

    // 2. Re-initialize all fields as in init_compiler
    comp->code = list_create(sizeof(uint8_t));
    comp->names = list_create(sizeof(String));

    comp->locals = stack_create(sizeof(local_t));
    comp->contexts = stack_create(sizeof(context_t));
    comp->loops = stack_create(sizeof(loop_t));
    comp->objects = stack_create(sizeof(String));
    comp->name = "";

    comp->current = create_context(false, comp->code, NULL);

    comp->instrs = ht_create(sizeof(list_t));

    comp->is_lookUp = false;
    comp->is_upvalue = false;
    comp->is_repl = false;

    push(comp->contexts, comp->current);
}
