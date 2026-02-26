#include <math.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "pi_vm.h"

#include "pi_opcode.h"
#include "pi_value.h"

#include "string.h"
#include "common.h"
#include "pi_func.h"
#include "gc.h"

#include "builtin/pi_builtin.h"

static PiMap *define_keys(vm_t *vm)
{
    table_t *table = ht_create(sizeof(Value));
    PiMap *keys_map = (PiMap *)new_map(table, true);

    // Letters A-Z
    for (char c = 'A'; c <= 'Z'; c++)
    {
        char keyname[2] = {c, '\0'};
        SDL_Scancode code = SDL_SCANCODE_A + (c - 'A');
        ht_put(table, keyname, &NEW_NUM(code));
    }

    // Digits 0-9
    for (char c = '0'; c <= '9'; c++)
    {
        char keyname[2] = {c, '\0'};
        SDL_Scancode code = SDL_SCANCODE_0 + (c - '0');
        ht_put(table, keyname, &NEW_NUM(code));
    }

    // Special keys
    struct
    {
        const char *name;
        SDL_Scancode code;
    } specials[] = {
        {"SPACE", SDL_SCANCODE_SPACE},
        {"ENTER", SDL_SCANCODE_RETURN},
        {"ESC", SDL_SCANCODE_ESCAPE},
        {"UP", SDL_SCANCODE_UP},
        {"DOWN", SDL_SCANCODE_DOWN},
        {"LEFT", SDL_SCANCODE_LEFT},
        {"RIGHT", SDL_SCANCODE_RIGHT},
        {"LSHIFT", SDL_SCANCODE_LSHIFT},
        {"RSHIFT", SDL_SCANCODE_RSHIFT},
        {"LCTRL", SDL_SCANCODE_LCTRL},
        {"RCTRL", SDL_SCANCODE_RCTRL},
        {"LALT", SDL_SCANCODE_LALT},
        {"RALT", SDL_SCANCODE_RALT},
    };

    for (int i = 0; i < sizeof(specials) / sizeof(specials[0]); i++)
        ht_put(table, specials[i].name, &NEW_NUM(specials[i].code));

    return keys_map;
}

/**
 * Initializes the virtual machine by allocating memory and
 * setting initial values for the program counter, stack pointer,
 * base pointer, and other components.
 */
vm_t *init_vm(compiler_t *comp, Screen *screen)
{

    // Allocate memory for the virtual machine instance
    vm_t *vm = (vm_t *)malloc(sizeof(vm_t));

    // Initialize program counter, stack pointer, and base pointer to 0
    vm->pc = 0;
    vm->sp = 0;
    vm->bp = 0;
    vm->ip = 0;

    // Set the code, constants, and names from the compiler to the VM
    vm->code = comp->code;
    vm->constants = comp->constants;
    vm->names = comp->names;
    vm->instrs = comp->instrs;

    // Create a hash table to store global variables
    vm->globals = ht_create(sizeof(Value));

    vm->objects = NULL;

    for (int i = 0; i < BUILTIN_CONST_COUNT; i++)
        ht_put(vm->globals, builtin_constants[i].name, &builtin_constants[i].value);

    for (int i = 0; i < BUILTIN_FUNC_COUNT; i++)
        ht_put(vm->globals, builtin_functions[i].name, new_native(builtin_functions[i].name, builtin_functions[i].func));

    vm->iter_sp = -1;
    vm->frame_sp = 0;

    vm->screen = screen;

    vm->running = true;

    vm->fps = TARGET_FPS;

    pthread_mutex_init(&vm->lock, NULL);

    mark_constants(vm);

    vm->counter = 0;

    vm->openUpvalues = NULL;

    vm->function = NULL;

    vm->next_gc = NEXT_GC;
    vm->obj_count = 0;

    vm->gc_stack = NULL;

    vm->cart = NULL;

    return vm;
}

/**
 * Resets an existing virtual machine to run new code.
 *
 * This function reinitializes the VM's execution state (PC, stack, etc.)
 * and loads new bytecode from a compiler. It intentionally preserves the
 * global variables table, allowing state to persist between script runs.
 *
 * @param vm The virtual machine instance to reset.
 * @param comp The compiler containing the new code to load.
 */
void vm_reset(vm_t *vm, compiler_t *comp)
{
    // Reset program counter, stack pointer, and base pointer to 0
    vm->pc = 0;
    vm->sp = 0;
    vm->bp = 0;
    vm->ip = 0;

    // Set the code, constants, and names from the compiler to the VM
    vm->code = comp->code;
    vm->constants = comp->constants;
    vm->names = comp->names;
    vm->instrs = comp->instrs;

    // Note: vm->globals is NOT reset. This is intentional to allow
    // persistence of global state between script executions in the shell.

    vm->iter_sp = -1;
    vm->frame_sp = 0;

    vm->running = true;

    // Reset GC stats to trigger collection sooner if needed
    vm->counter = 0;
    vm->next_gc = NEXT_GC;

    vm->openUpvalues = NULL;
    vm->function = NULL;

    // Mark new constants from the new compiler for GC
    mark_constants(vm);
}

/**
 * Adds an object to the VM's object list.
 *
 * This function takes in a newly allocated object and adds it to the
 * front of the list of objects in the virtual machine. It returns the
 * newly added object.
 *
 * @param vm The virtual machine instance.
 * @param obj The object to add to the object list.
 * @return The newly added object.
 */
inline Object *add_obj(vm_t *vm, Object *obj)
{
    if (obj->in_gcList)
        return obj; // Already in the list, skip

    // Mark as added
    obj->in_gcList = true;

    obj->gc_color = GC_WHITE; // New objects start as white

    // Add to the front of the list
    obj->next = vm->objects;
    vm->objects = obj;

    return obj;
}

/**
 * Counts the number of objects in the virtual machine's object list.
 *
 * This function iterates over the linked list of objects and returns the
 * total count of objects in the list. It is used for debugging purposes
 * to track the number of objects in use.
 *
 * @param vm The virtual machine instance.
 * @return The number of objects in the object list.
 */
static inline int count_objs(vm_t *vm)
{
    int count = 0;
    Object *obj = vm->objects;
    while (obj)
    {
#ifdef DEBUG
        // Print debugging information about the object
        printf("[DEBUG] Counting object at %p\n", (void *)obj);
#endif
        count++;
        obj = obj->next;
    }
    return count;
}

/**
 * Reports a virtual machine error with a specified message.
 *
 * This function outputs an error message to the standard error stream,
 * indicating a critical error in the virtual machine operation. It attempts
 * to provide context by displaying the line number and function name where
 * the error occurred, if available. The program will terminate immediately
 * after displaying the error message.
 *
 * @param vm The virtual machine instance containing execution information.
 * @param message The error message to be displayed.
 */

void vm_error(vm_t *vm, const char *message)
{
    instr_t *instr = NULL;
    char *name = "<global>";

    if (vm->frame_sp > 0)
    {
        Frame *top = vm->frames[vm->frame_sp - 1];
        name = top->function->name;
    }

    list_t *instrs = ht_get(vm->instrs, name);
    int size = instrs ? list_size(instrs) : 0;

    for (int i = 0; i < size; i++)
    {
        instr_t *cur = (instr_t *)list_getAt(instrs, i);

        if (cur->offset > vm->pc)
            break;
        instr = cur;
    }

    if (global_errorHandler)
    {
        char buffer[1024];
        if (instr && instr->fun_name)
            snprintf(buffer, sizeof(buffer), "%s (in function '%s')", message, instr->fun_name);
        else
            snprintf(buffer, sizeof(buffer), "%s", message);

        global_errorHandler(buffer, instr ? instr->line : -1, 0);
        return;
    }

    if (instr)
    {
        fprintf(stderr, "\n\033[1;31m[RUNTIME ERROR] at line %d", instr->line);
        if (instr->fun_name)
            fprintf(stderr, " in function '%s'", instr->fun_name);
        fprintf(stderr, ":\033[0m %s\n\n", message);
    }
    else
        fprintf(stderr, "\n\033[1;31m[RUNTIME ERROR] at unknown location:\033[0m %s\n\n", message);

    exit(EXIT_FAILURE);
}

/**
 * Reports a virtual machine error with a formatted message.
 *
 * This function constructs a formatted error message using a variable
 * argument list and passes it to the vm_error function for reporting.
 *
 * @param vm The virtual machine instance containing execution information.
 * @param fmt The format string for the error message.
 * @param ... The variable arguments to be formatted into the message.
 */
void vm_errorf(vm_t *vm, const char *fmt, ...)
{
    char buffer[1024]; // Buffer to hold the formatted error message
    va_list args;

    // Initialize the variable argument list
    va_start(args, fmt);

    // Format the error message into the buffer
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    // Clean up the variable argument list
    va_end(args);

    // Report the formatted error message
    vm_error(vm, buffer);
}

/**
 * Pops a value from the stack and returns it.
 *
 * This function retrieves the top element from the stack and
 * decrements the stack pointer. If the stack is empty, it will
 * raise an error.
 *
 * @return A Value object representing the popped value.
 */
static inline Value pop_stack(vm_t *vm)
{
    if (vm->sp <= 0)
        vm_error(vm, "Stack underflow: Attempted to pop from an empty stack");

    return vm->stack[--vm->sp];
}

/**
 * Pushes a value onto the stack.
 *
 * This function adds a new value to the top of the stack and increments the
 * stack pointer. If the stack is full, it will not push the value and instead
 * raise an error.
 *
 * @param value The value to be pushed onto the stack.
 */
static inline void push_stack(vm_t *vm, Value value)
{
    if (vm->sp >= STACK_MAX)
        vm_error(vm, "Stack overflow: Attempted to push onto a full stack");

    vm->stack[vm->sp++] = value;
}

/**
 * Peeks at the top element on the stack without modifying the stack pointer.
 *
 * This function returns the top element from the stack without modifying the
 * stack pointer. If the stack is empty, it will raise an error.
 *
 * @return A Value object representing the top value on the stack.
 */
static inline Value peek_stack(vm_t *vm)
{
    if (vm->sp <= 0)
        vm_error(vm, "Stack underflow: Attempted to peek at an empty stack");

    return vm->stack[vm->sp - 1];
}

/**
 * Checks if the stack is empty relative to the current base pointer.
 *
 * This function compares the stack pointer to the base pointer. If the stack
 * pointer is equal to the base pointer, it means that the stack is empty.
 *
 * @return true if the stack is empty, false otherwise
 */
static bool stack_isEmpty(vm_t *vm)
{
    return vm->sp == vm->bp;
}

/**
 * Pushes a frame onto the stack.
 *
 * This function increments the frame stack pointer and assigns the
 * given frame to the frame stack at the new index. If the frame
 * stack is full, it will raise an error.
 *
 * @param vm The virtual machine instance.
 * @param frame The frame to push onto the stack.
 */
void push_frame(vm_t *vm, Frame *frame)
{
    if (vm->frame_sp >= STACK_MAX)
        vm_error(vm, "Stack overflow: Attempted to push onto a full stack");

    vm->frames[vm->frame_sp++] = frame;
}

/**
 * Pops a frame from the stack.
 *
 * This function retrieves the top element from the stack and decrements the
 * frame stack pointer. If the stack is empty, it will raise an error.
 *
 * @return A Frame object representing the popped frame.
 */
Frame *pop_frame(vm_t *vm)
{
    if (vm->frame_sp <= 0)
        vm_error(vm, "Stack underflow: Attempted to pop from an empty stack");

    Frame *frame = vm->frames[--vm->frame_sp];
    return frame;
}

/**
 * Reads a name from the list of names stored in the virtual machine.
 *
 * @param index The index of the name to read from the list of names.
 * @return A C string containing the name at the specified index.
 */
static inline char *read_name(vm_t *vm, int index)
{
    return string_get(vm->names, index);
}

/**
 * Checks if the given value is considered false.
 *
 * This function is used to compare a value to a boolean false value.
 * It checks if the value is NULL or if the type of the value is NIL.
 *
 * @param value The value to check.
 * @return true if the value is false, false otherwise.
 */
static inline bool is_false(vm_t *vm, Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static inline int read_short(vm_t *vm)
{
    uint8_t *code = (uint8_t *)vm->code->data; // Access the bytecode from the VM's code
    int high = code[vm->pc++] & 0xFF;          // Get the high byte and mask it
    int low = code[vm->pc++] & 0xFF;           // Get the low byte and mask it

    return (high << 8) | low; // Combine high and low bytes into a 16-bit short
}

static inline int _read_short(uint8_t *code, int pc)
{
    int high = code[pc] & 0xFF;    // Get the high byte and mask it
    int low = code[pc + 1] & 0xFF; // Get the low byte and mask it
    return (high << 8) | low;      // Combine high and low bytes into a 16-bit short
}

static UpValue *capture_upvalue(vm_t *vm, int index)
{
    UpValue *prev = NULL;
    UpValue *upvalue = vm->openUpvalues;

    while (upvalue != NULL && upvalue->index != index)
    {
        prev = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->index == index)
        return upvalue;

    UpValue *_upvalue = (UpValue *)malloc(sizeof(UpValue));
    _upvalue->value = vm->stack[index]; // Reference stack value
    _upvalue->index = index;

    _upvalue->next = upvalue;
    if (prev == NULL)
        vm->openUpvalues = _upvalue;
    else
        prev->next = _upvalue;
    return _upvalue;
}

static void remove_upvalue(vm_t *vm, int index)
{
    UpValue *prev = NULL;
    UpValue *upvalue = vm->openUpvalues;

    while (upvalue != NULL && upvalue->index != index)
    {
        prev = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->index == index)
    {
        upvalue->index = -1;
        upvalue->value = vm->stack[index];

        if (prev == NULL)
            vm->openUpvalues = upvalue->next;
        else
            prev->next = upvalue->next;
    }
}

/**
 * Bind a function to an instance. The function is returned as a new function
 * with the first argument set to the instance.
 *
 * @param function The function to bind.
 * @param instance The instance to bind to.
 * @return A new function bound to the given instance.
 */
static Value bind(vm_t *vm, Function *function, Object *instance)
{
    // Copy the function object to keep the original intact
    Object *fn = new_func(function->name, function->body,
                          function->params, NULL, instance);

    // Set the is_method flag to true
    ((Function *)fn)->is_method = true;
    add_obj(vm, fn); // Critical - adds to GC tracking

    // Return the new function
    return NEW_OBJ(fn);
}

/**
 * Constructs a new object instance from a given prototype map.
 *
 * This function creates a new map instance, setting the original map as its
 * prototype and copying over its key-value pairs. If a key holds a function,
 * it is bound to the new instance. The constructor function is called if it exists.
 *
 * @param vm The virtual machine instance.
 * @param map The prototype map from which to construct the object.
 * @param argc The number of arguments provided for the constructor.
 * @param argv The arguments to pass to the constructor.
 * @return A new object instance.
 */
static Object *construct(vm_t *vm, PiMap *map, size_t argc, Value *argv)
{
    // Create a new table for the instance
    table_t *table = ht_create(sizeof(Value));
    char **keys = ht_keys(map->table);
    int size = ht_length(map->table);

    // Create a new map instance and set its prototype
    Object *instance = new_map(table, true);

    ((PiMap *)instance)->proto = map;

    // Iterate over the keys in the prototype map
    for (size_t i = 0; i < size; i++)
    {

        char *key = keys[i];
        if (strcmp(key, "constructor") != 0) // Skip the constructor key
        {
            Value value = *(Value *)ht_get(map->table, key);
            if (IS_FUN(value))
            {
                // Bind function to the new instance
                Value fn = bind(vm, AS_FUN(value), instance);
                ht_put(table, key, &fn);
            }
            else
                // Copy non-function values directly
                ht_put(table, key, ht_get(map->table, key));
        }
    }

    // Push the new instance onto the VM stack
    // vm->stack[vm->sp] = NEW_OBJ(instance);

    // Prepare arguments with 'this' as the first argument for the constructor
    Value *fargs = (Value *)malloc(sizeof(Value) * (argc + 1));
    fargs[0] = NEW_OBJ(instance); // 'this' reference
    memcpy(fargs + 1, argv, sizeof(Value) * argc);

    // Invoke the constructor if it exists
    void *item = ht_get(map->table, "constructor");
    Value constructor = item ? *(Value *)item : NEW_NIL();

    if (IS_FUN(constructor))
    {
        AS_FUN(constructor)->is_method = false; // Ensure it's not a method
        instance = AS_OBJ(call_func(vm, AS_FUN(constructor), argc + 1, fargs));
    }

    // Free the allocated arguments array
    free(fargs);
    return instance;
}

void run(vm_t *vm)
{
    int length = vm->code->size;
    int pc = vm->pc;

    uint8_t op;
    uint16_t index;
    int address;

    uint8_t *code = (uint8_t *)vm->code->data;

    Value value;

    Value nilValue;

    Object *iter = NULL;

    UpValue *upValue;

    Function *function = (Function *)vm->function;

    while (pc < length && vm->running)
    {

        op = code[pc++];
        vm->counter++;

        vm->ip++; // Advance instruction index

        // printf("OP: %d, PC: %d, IP: %d\n", op, pc, vm->ip);

        // Cast the opcode to the OpCode enum
        switch ((OpCode)op)
        {
        case OP_LOAD_CONST:
        {
            // Read a two-byte short value from the bytecode to get the constant index
            index = (code[pc++] << 8);
            index |= code[pc++];
            // Get the constant from the constants list using the index
            Value constant = *(Value *)list_getAt(vm->constants, index);

            // Push the constant onto the stack
            push_stack(vm, constant);

            break;
        }

        case OP_STORE_GLOBAL:
        {
            index = code[pc++];
            char *name = read_name(vm, index);

            Value _newValue = pop_stack(vm);
            ht_put(vm->globals, name, &_newValue); // Store directly, no malloc!

            break;
        }

        case OP_LOAD_GLOBAL:
        {
            index = code[pc++];
            char *name = string_get(vm->names, index);
            Value *_value = ht_get(vm->globals, name);
            if (_value == NULL)
            {
                nilValue = NEW_NIL();
                _value = &nilValue;
            }
            push_stack(vm, *_value);
            break;
        }

        case OP_LOAD_LOCAL:
        {
            op = code[pc++];
            Value value = vm->stack[vm->bp + op];
            push_stack(vm, value);
            break;
        }

        case OP_STORE_LOCAL:
        {
            op = code[pc++];
            vm->stack[vm->bp + op] = pop_stack(vm);
            break;
        }

        case OP_POP:
        {
            remove_upvalue(vm, vm->sp - 1);
            Value value = pop_stack(vm);
            break;
        }
        case OP_POP_N:
        {
            op = code[pc++];
            for (int i = 0; i < op; i++)
            {
                remove_upvalue(vm, vm->sp - 1);
                pop_stack(vm);
            }
        }
        break;

        case OP_DUP_TOP:
            push_stack(vm, peek_stack(vm));
            break;

        case OP_JUMP_IF_FALSE:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]); // Signed 16-bit offset

            Value value = pop_stack(vm);
            if (!as_bool(value))
                pc += offset - 1; // relative jump
            else
                pc += 2;
            break;
        }

        case OP_JUMP:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]); // Signed 16-bit offset
            pc += offset - 1;
            break;
        }

        case OP_JUMP_IF_TRUE:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]); // Signed 16-bit offset

            Value value = pop_stack(vm);
            if (as_bool(value))
                pc += offset - 1; // relative jump
            else
                pc += 2;
            break;
        }

        case OP_COMPARE:
        {
            uint8_t op = code[pc++];

            Value right = pop_stack(vm);
            Value left = pop_stack(vm);

            bool result = false;
            int cmp = compare(left, right);

            switch (op)
            {
            case 0: // "=="
                result = (cmp == 0);
                break;
            case 1: // "!="
                result = (cmp != 0);
                break;
            case 2: // ">"
                result = (cmp > 0);
                break;
            case 3: // "<"
                result = (cmp < 0);
                break;
            case 4: // ">="
                result = (cmp >= 0);
                break;
            case 5: // "<="
                result = (cmp <= 0);
                break;
            default:
                vm_errorf(vm, "Unknown opcode: [%d]", op);
            }
            push_stack(vm, NEW_BOOL(result));

            break;
        }
        case OP_BINARY:
        {
            uint8_t op = code[pc++];

            Value right = pop_stack(vm);
            Value left = pop_stack(vm);

            switch (op)
            {
            case 0: // "+"
            {
                if (is_numeric(left) && is_numeric(right))
                {
                    push_stack(vm, NEW_NUM(as_number(left) + as_number(right)));
                    break;
                }

                if (IS_STRING(left) || IS_STRING(right))
                {
                    // Coerce both to strings
                    char *l_str = as_string(left);
                    char *r_str = as_string(right);

                    size_t len = strlen(l_str) + strlen(r_str) + 1;
                    char *res = (char *)malloc(len);
                    if (!res)
                        vm_error(vm, "Memory allocation failed.");

                    strcpy(res, l_str);
                    strcat(res, r_str);

                    push_stack(vm, NEW_OBJ(add_obj(vm, new_pistring(res))));

                    free(l_str);
                    free(r_str);
                    break;
                }

                if (IS_LIST(left))
                {
                    PiList *list = AS_LIST(left);
                    list_add(list->items, &right);

                    // --- Matrix integrity check ---
                    if (list->rows == 1 && list->cols >= 0)
                    {
                        // Originally a 1xN matrix, now N+1
                        if (!IS_NUM(right))
                        {
                            list->rows = -1;
                            list->cols = -1;
                            list->is_numeric = false;
                        }
                        else
                            list->cols++; // still a row vector
                    }
                    else if (list->rows > 1 && list->cols > 0)
                    {
                        // Originally NxM matrix
                        if (!IS_LIST(right))
                        {
                            list->rows = -1;
                            list->cols = -1;
                            list->is_numeric = false;
                        }
                        else
                        {
                            PiList *_list = (PiList *)AS_OBJ(right);
                            if (!_list->is_numeric || _list->items->size != list->cols)
                            {
                                list->rows = -1;
                                list->cols = -1;
                                list->is_numeric = false;
                            }
                            else
                                list->rows++; // still an NxM matrix
                        }
                    }
                    else
                    {
                        // Not originally a matrix, check if it can now become one
                        if (list->items->size == 1 && IS_NUM(right) && IS_NUM(((Value *)list->items->data)[0]))
                        {
                            list->is_numeric = true;
                            list->rows = 1;
                            list->cols = 2;
                        }
                    }

                    push_stack(vm, left);
                    break;
                }
                if (IS_NAN(left) || IS_NAN(right))
                {
                    push_stack(vm, NEW_NUM(NAN));
                    break;
                }
                vm_error(vm, "Unsupported operand types for binary operator [+].");
            }
            case 1: // "-"
            {
                if (is_numeric(left) && is_numeric(right))
                {
                    push_stack(vm, NEW_NUM(as_number(left) - as_number(right)));
                    break;
                }

                if (IS_OBJ(left))
                {
                    if (IS_LIST(left))
                    {
                        PiList *list = AS_LIST(left);
                        for (int i = 0; i < list_size(list->items); i++)
                        {
                            Value item = *(Value *)list_getAt(list->items, i);
                            if (equals(item, right))
                            {
                                list_remove(list->items, i);
                                break;
                            }
                        }
                        push_stack(vm, left);
                        break;
                    }

                    if (IS_STRING(left))
                    {
                        char *l_str = as_string(left);
                        char *r_str = as_string(right);

                        size_t l_len = strlen(l_str);
                        size_t r_len = strlen(r_str);

                        char *res = (char *)malloc(l_len + 1); // Worst case

                        char *w_ptr = res;
                        char *r_ptr = l_str;
                        char *match;

                        while ((match = strstr(r_ptr, r_str)) != NULL)
                        {
                            size_t chunk_len = match - r_ptr;
                            memcpy(w_ptr, r_ptr, chunk_len);
                            w_ptr += chunk_len;
                            r_ptr = match + r_len;
                        }

                        strcpy(w_ptr, r_ptr); // copy the tail

                        push_stack(vm, NEW_OBJ(add_obj(vm, new_pistring(res))));

                        free(res);
                        free(l_str);
                        free(r_str);
                        break;
                    }

                    vm_error(vm, "Unsupported operand types for binary operator [-].");
                }

                vm_error(vm, "Unsupported operand types for binary operator [-].");
            }
            break;
            case 2: // "*"
            {
                if (is_numeric(left))
                    // Multiply two numbers
                    push_stack(vm, NEW_NUM(as_number(left) * as_number(right)));
                else if (left.type == VAL_OBJ)
                {
                    if (IS_LIST(left) && IS_LIST(right))
                    {
                        PiList *A = AS_LIST(left);
                        PiList *B = AS_LIST(right);

                        if (!A->is_numeric || !B->is_numeric)
                            vm_error(vm, "Matrix multiplication requires numeric lists.");

                        if (A->cols == -1 || B->cols == -1)
                            vm_error(vm, "Matrix dimensions are not set properly.");

                        if (A->cols != B->rows)
                            vm_error(vm, "Matrix multiplication dimension mismatch.");

                        int m = A->rows;
                        int n = A->cols;
                        int p = B->cols;

                        list_t *result = list_create(sizeof(Value));

                        for (int i = 0; i < m; i++)
                        {
                            Value *rowA_val = (Value *)list_getAt(A->items, i);
                            list_t *rowA = as_list(*rowA_val);
                            list_t *temp = list_create(sizeof(Value));

                            for (int j = 0; j < p; j++)
                            {
                                double sum = 0.0;

                                for (int k = 0; k < n; k++)
                                {
                                    // Get A[i][k]
                                    Value *a_val = (Value *)list_getAt(rowA, k);
                                    double a = as_number(*a_val);

                                    // Get B[k][j]
                                    Value *rowB_val = (Value *)list_getAt(B->items, k);
                                    list_t *rowB = as_list(*rowB_val);
                                    Value *b_val = (Value *)list_getAt(rowB, j);
                                    double b = as_number(*b_val);

                                    sum += a * b;
                                }

                                list_add(temp, &NEW_NUM(sum));
                            }

                            list_add(result, &NEW_OBJ(new_list(temp)));
                        }

                        Object *res_obj = add_obj(vm, new_list(result));
                        ((PiList *)res_obj)->is_numeric = true;
                        ((PiList *)res_obj)->rows = m;
                        ((PiList *)res_obj)->cols = p;
                        push_stack(vm, NEW_OBJ(res_obj));
                        break;
                    }
                    else if (IS_LIST(left))
                    {
                        int count = (int)as_number(right); // Assuming `right` is a number
                        list_t *list = as_list(left);      // Assuming `as_list` returns a `list_t *`

                        list_t *result = list_create(list->i_size);
                        for (int i = 0; i < count; i++)
                            list_addAll(result, list);

                        Object *_result = new_list(result);
                        if (AS_LIST(left)->is_numeric)
                            ((PiList *)_result)->is_numeric = true;

                        push_stack(vm, NEW_OBJ(add_obj(vm, _result))); // Assuming `new_list` creates a `Value` with type `OBJ_LIST`
                    }
                    else if (IS_STRING(left))
                    {
                        int count = (int)as_number(right); // Assuming `right` is a number
                        // the original strings
                        char *str = as_string(left);
                        // original string length
                        size_t o_len = strlen(str);
                        // result string length
                        size_t r_len = o_len * count;

                        // allocate memory for the result string
                        char *result = (char *)malloc(r_len + 1); // Allocate space for the repeated string
                        result[0] = '\0';

                        for (int i = 0; i < count; i++)
                            strcat(result, str);

                        push_stack(vm, NEW_OBJ(add_obj(vm, new_pistring(result))));
                        free(result); // Clean up temporary string buffer
                    }
                    else
                        vm_error(vm, "Unsupported operand types for binary operator [*].");
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [*].");

                break;
            }
            case 3: // "/"
            {
                double denominator = as_number(right);

                if (denominator == 0.0)
                {
                    push_stack(vm, NEW_NUM(INFINITY)); // Push infinity to indicate undefined result
                    break;
                }

                double numerator = as_number(left);
                push_stack(vm, NEW_NUM(numerator / denominator));
                break;
            }
            case 4: // "%"
            {
                double denominator = as_number(right);

                if ((int)denominator == 0) // If denominator is zero, return NaN
                    push_stack(vm, NEW_NAN());
                else
                    push_stack(vm, NEW_NUM((int)as_number(left) % (int)denominator));
                break;
            }
            case 5: // "&&"
                push_stack(vm, NEW_BOOL(as_bool(left) && as_bool(right)));
                break;
            case 6: // "||"
                push_stack(vm, NEW_BOOL(as_bool(left) || as_bool(right)));
                break;
            case 7: // "**"
                push_stack(vm, NEW_NUM(pow(as_number(left), as_number(right))));
                break;
            case 8: // "&"
            {
                if (is_numeric(left))
                    push_stack(vm, NEW_NUM((int)as_number(left) & (int)as_number(right)));
                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    int _right = (int)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((int)as_number(item) & _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [&].");

                break;
            }

            case 9: // "|"
            {
                if (is_numeric(left))
                    push_stack(vm, NEW_NUM((int)as_number(left) | (int)as_number(right)));
                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    int _right = (int)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((int)as_number(item) | _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [|].");

                break;
            }

            case 10: // "^"
            {

                if (IS_LIST(left) && IS_LIST(right))
                {
                    PiList *l_list = AS_LIST(left);
                    PiList *r_list = AS_LIST(right);

                    if (!l_list->is_numeric || !r_list->is_numeric)
                        vm_error(vm, "Cross product requires numeric lists.");

                    if (list_size(l_list->items) != 3 || list_size(r_list->items) != 3)
                        vm_error(vm, "Cross product is defined for 3-dimensional vectors only.");

                    Value *a = l_list->items->data;
                    Value *b = r_list->items->data;

                    double x = as_number(a[1]) * as_number(b[2]) - as_number(a[2]) * as_number(b[1]);
                    double y = as_number(a[2]) * as_number(b[0]) - as_number(a[0]) * as_number(b[2]);
                    double z = as_number(a[0]) * as_number(b[1]) - as_number(a[1]) * as_number(b[0]);

                    list_t *res = list_create(sizeof(Value));
                    list_add(res, &NEW_NUM(x));
                    list_add(res, &NEW_NUM(y));
                    list_add(res, &NEW_NUM(z));

                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(res))));
                    break;
                }
                else if (is_numeric(left))
                    push_stack(vm, NEW_NUM((int)as_number(left) ^ (int)as_number(right)));

                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    int _right = (int)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((int)as_number(item) ^ _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [^].");

                break;
            }

            case 11: // "<<"
            {
                if (is_numeric(left))
                    push_stack(vm, NEW_NUM((int)as_number(left) << (int)as_number(right)));

                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    int _right = (int)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((int)as_number(item) << _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [<<].");

                break;
            }

            case 12: // ">>"
            {
                if (is_numeric(left))
                    push_stack(vm, NEW_NUM((int)as_number(left) >> (int)as_number(right)));

                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    int _right = (int)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((int)as_number(item) >> _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [>>].");

                break;
            }

            case 13: // ">>>"
            {
                if (is_numeric(left))
                    push_stack(vm, NEW_NUM((uint32_t)as_number(left) >> (uint32_t)as_number(right)));

                else if (left.type == VAL_OBJ && OBJ_TYPE(left) == OBJ_LIST)
                {
                    list_t *list = as_list(left);
                    list_t *result = list_create(sizeof(Value));

                    uint32_t _right = (uint32_t)as_number(right);

                    for (int i = 0; i < list_size(list); i++)
                    {
                        Value item = *(Value *)list_getAt(list, i);
                        list_add(result, &NEW_NUM((uint32_t)as_number(item) >> _right));
                    }
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [>>>].");

                break;
            }

            case 14: // "." (dot product)
            {
                if (IS_LIST(left) && IS_LIST(right))
                {
                    PiList *l_list = AS_LIST(left);
                    PiList *r_list = AS_LIST(right);

                    if (!l_list->is_numeric || !r_list->is_numeric)
                        vm_error(vm, "Dot product requires numeric lists.");

                    int l_size = list_size(l_list->items);
                    int r_size = list_size(r_list->items);

                    if (l_size != r_size)
                        vm_error(vm, "Dot product requires lists of the same length.");

                    double result = 0;
                    for (int i = 0; i < l_size; i++)
                    {
                        Value a = *(Value *)list_getAt(l_list->items, i);
                        Value b = *(Value *)list_getAt(r_list->items, i);
                        result += as_number(a) * as_number(b);
                    }
                    push_stack(vm, NEW_NUM(result));
                    break;
                }
                vm_error(vm, "Unsupported operand types for binary operator [.]");
            }

            case 15: // instance of operator [is]
            {

                if (!IS_MAP(left) || !IS_MAP(right))
                {
                    push_stack(vm, NEW_BOOL(false));
                    break;
                }

                Object *inst_obj = AS_OBJ(left);
                Object *proto_obj = AS_OBJ(right);

                if (inst_obj->type != OBJ_MAP || proto_obj->type != OBJ_MAP)
                {
                    push_stack(vm, NEW_BOOL(false));
                    break;
                }

                PiMap *map = (PiMap *)inst_obj;
                PiMap *proto = (PiMap *)proto_obj;

                // Traverse the prototype chain
                while (map != NULL)
                {
                    if (map == proto)
                    {
                        push_stack(vm, NEW_BOOL(true));
                        break;
                    }
                    map = map->proto;
                }

                if (!map)
                    push_stack(vm, NEW_BOOL(false));

                break;
            }

            break;
            }
            break;
        }
        case OP_UNARY:
        {

            uint8_t op = code[pc++];       // Get the unary operation code
            Value operand = pop_stack(vm); // Get the operand from the stack

            switch (op)
            {
            case 0: // Unary plus
                push_stack(vm, NEW_NUM(as_number(operand)));
                break;

            case 1: // Unary minus
                push_stack(vm, NEW_NUM(-as_number(operand)));
                break;

            case 2: // Logical NOT
                push_stack(vm, NEW_BOOL(!as_bool(operand)));
                break;

            case 3: // Bitwise NOT
                push_stack(vm, NEW_NUM(~(int)as_number(operand)));
                break;

            case 4: // Collection size
            {
                if (IS_COLLECTION(operand))
                {
                    switch (OBJ_TYPE(operand))
                    {
                    case OBJ_LIST:
                        push_stack(vm, NEW_NUM(list_size(AS_LIST(operand)->items)));
                        break;
                    case OBJ_STRING:
                        push_stack(vm, NEW_NUM(AS_STRING(operand)->length));
                        break;
                    case OBJ_MAP:
                        push_stack(vm, NEW_NUM(map_size(AS_MAP(operand))));
                        break;
                    }
                }
                else
                    vm_error(vm, "Unsupported operand type for '#' operator.");

                break;
            }
            case 5: // "++"
                push_stack(vm, NEW_NUM(as_number(operand) + 1));
                break;

            case 6: // "--"
                push_stack(vm, NEW_NUM(as_number(operand) - 1));
                break;

            default:
                vm_error(vm, "Unknown unary operator.");
            }

            break;
        }
        case OP_CALL_FUNCTION:
        {

            // Read the number of arguments from the bytecode
            uint8_t num_args = code[pc++];

            // Allocate memory for the arguments
            Value args[num_args];

            // Pop the arguments off the VM's stack in reverse order.
            for (int i = num_args - 1; i >= 0; i--)
                args[i] = pop_stack(vm);

            // Pop the function (callee) from the stack.
            Value callee = pop_stack(vm);

            if (IS_FUN(callee))
            {

                vm->function = AS_OBJ(callee);

                vm->pc = pc;
                // Call native function if it's a built-in
                Value result = call_func(vm, AS_FUN(callee), num_args, args);
                if (IS_OBJ(result))
                    add_obj(vm, AS_OBJ(result));
                push_stack(vm, result);
            }
            else if (IS_MAP(callee))
            {
                PiMap *map = AS_MAP(callee);
                if (map->is_instance)
                    vm_error(vm, "Attempt to call an Object instance.");
                else
                    push_stack(vm, NEW_OBJ(add_obj(vm, construct(vm, map, num_args, args))));
            }
            else
                vm_error(vm, "Attempt to call a non-function object.");

            break;
        }

        case OP_PUSH_ITER:
        {
            // Pop the iterable object from the stack
            Value iterable = pop_stack(vm);

            // Ensure the object is iterable
            if (!IS_OBJ(iterable) || !is_iterable(AS_OBJ(iterable)))
                vm_error(vm, "Error: Object is not iterable.");

            iter = AS_OBJ(iterable);

            // Reset iterator state (common for all iterators)
            iter_reset(iter);

            // Push the iterator onto the iterator stack
            vm->iters[++vm->iter_sp] = iter; // Push a pointer to the iterator
            break;
        }

        case OP_LOOP:
        {
            // Read the jump address from the bytecode
            uint16_t address = (code[pc] << 8);
            address |= code[pc + 1];

            // Get the current iterator from the top of the stack
            if (vm->iter_sp == -1)
                vm_error(vm, "Error: No active iterator.");

            iter = vm->iters[vm->iter_sp];

            // Check if the iterator has more elements
            if (iter_hasNext(iter))
            {
                if (iter->type == OBJ_MAP)
                {
                    PiMap *map = (PiMap *)iter;
                    ht_next(&map->it);
                    char *key = map->it.key;
                    push_stack(vm, NEW_OBJ(add_obj(vm, new_pistring(key))));
                }
                else
                {
                    // Get the next value from the iterator
                    Value value = iter_next(iter);
                    // TODO: check me in the future
                    // if (IS_OBJ(value))
                    //     add_obj(vm, AS_OBJ(value));
                    push_stack(vm, value);
                }
                pc += 2;
            }
            else
            {
                // Iterator exhausted; pop it from the stack
                vm->iter_sp--;

                // Jump to the specified address
                pc += address - 1;
            }
            break;
        }

        case OP_POP_ITER:
        {
            if (vm->iter_sp != -1)
                iter = vm->iters[vm->iter_sp--];
            // Perform cleanup if needed
            break;
        }
        case OP_PUSH_RANGE:
        {
            // Pop the range values from the stack
            Value step = pop_stack(vm);
            Value end = pop_stack(vm);
            Value start = pop_stack(vm);

            if (!IS_NUM(start) || !IS_NUM(end))
                vm_error(vm, "PiRange `start` and `end` must be numbers.");

            // Create a new range object
            if (!IS_NIL(step) && !IS_NUM(step))
                vm_error(vm, "PiRange `step` must be nil or a number.");
            else
            {

                // Extract numerical values
                double _start = as_number(start);
                double _end = as_number(end);
                double _step;
                if (IS_NIL(step))
                    _step = (_start < _end) ? 1.0 : -1.0;
                else
                    _step = as_number(step);
                Object *range = add_obj(vm, new_range(_start, _end, _step));
                push_stack(vm, NEW_OBJ(range)); // Push the range onto the stack
            }

            break;
        }

        case OP_PUSH_LIST:
        {
            int numElements = (code[pc++] << 8) | code[pc++];
            list_t *list = list_create(sizeof(Value));

            if (numElements == 0)
            {
                Object *l_obj = add_obj(vm, new_list(list));
                PiList *plist = (PiList *)l_obj;
                plist->is_numeric = true;
                plist->is_matrix = false;
                plist->rows = 0;
                plist->cols = 0;
                push_stack(vm, NEW_OBJ(l_obj));
                break;
            }

            vm->sp -= numElements;

            bool is_numeric = true;
            bool is_matrix = true;
            int rows = -1, cols = -1;

            // First: collect all values and add to list
            for (int i = 0; i < numElements; i++)
            {
                Value v = vm->stack[vm->sp + i];
                if (is_numeric && !IS_NUM(v))
                    is_numeric = false;
                list_add(list, &v);
            }

            if (is_numeric)
            {
                is_matrix = false;
                rows = 1;
                cols = numElements;
            }
            else
            {
                // check for matrix: list of equal-sized numeric lists
                Value first = vm->stack[vm->sp];
                if (IS_LIST(first))
                {
                    PiList *pl0 = (PiList *)AS_OBJ(first);
                    if (pl0->is_numeric)
                    {
                        cols = pl0->items->size;
                        rows = numElements;
                        for (int i = 0; i < numElements; i++)
                        {
                            Value v = vm->stack[vm->sp + i];
                            if (!IS_LIST(v))
                            {
                                is_matrix = false;
                                break;
                            }
                            PiList *pl = (PiList *)AS_OBJ(v);
                            if (!pl->is_numeric || pl->items->size != cols)
                            {
                                is_matrix = false;
                                break;
                            }
                        }
                    }
                    else
                        is_matrix = false;
                }
                else
                    is_matrix = false;
            }

            Object *l_obj = add_obj(vm, new_list(list));
            PiList *plist = (PiList *)l_obj;
            plist->is_numeric = is_numeric;
            plist->is_matrix = is_matrix;
            plist->rows = is_matrix ? rows : -1;
            plist->cols = is_matrix ? cols : -1;

            push_stack(vm, NEW_OBJ(l_obj));
            break;
        }

        case OP_PUSH_MAP:
        {

            // Read the number of elements in the map
            int numElements = code[pc++] << 8;
            numElements |= code[pc++];
            // create a new hashtable
            table_t *table = ht_create(sizeof(Value));

            // Adjust the stack pointer to the first element of the map
            int _sp = vm->sp - (numElements * 2);

            // Populate the map directly from the stack
            for (int i = _sp; i < vm->sp; i += 2)
            {
                Value value = vm->stack[i];

                char *key = AS_CSTRING(vm->stack[i + 1]);
                if (IS_FUN(value))
                    AS_FUN(value)->is_method = true;

                ht_put(table, key, &value);
            }

            vm->sp = _sp;

            // Push the new map onto the stack
            Object *map = add_obj(vm, new_map(table, false));
            push_stack(vm, NEW_OBJ(map));

            break;
        }

        case OP_PUSH_FUNCTION:
        {
            // Read the number of parameters
            int numParams = code[pc++];

            ObjCode *body = AS_CODE(pop_stack(vm));
            char *name = AS_CSTRING(pop_stack(vm));

            list_t *defaults = list_create(sizeof(Value));

            // Adjust the stack pointer to the first parameter
            vm->sp -= numParams;

            // Populate the parameter list directly from the stack
            for (int i = 0; i < numParams; i++)
            {
                Value param = vm->stack[vm->sp + i];
                list_add(defaults, &param);
            }

            // Create a new function object
            Object *function = new_func(name, body, defaults, NULL, NULL);

            // Push the new function onto the stack
            push_stack(vm, NEW_OBJ(add_obj(vm, function)));

            break;
        }

        case OP_PUSH_CLOSURE:
        {
            int numParams = code[pc++];
            // Read the number of upvalues
            int numUpvalues = code[pc++];

            UpValue **upvalues = ALLOCATE(UpValue *, numUpvalues);

            // Populate the upvalue list directly from the stack
            for (int i = 0; i < numUpvalues; i++)
            {
                bool is_local = as_bool(pop_stack(vm));
                int index = as_number(pop_stack(vm));
                UpValue *upvalue;

                if (is_local)
                    upvalue = capture_upvalue(vm, vm->bp + index);
                else
                    upvalue = function->upvalues[index];

                upvalues[numUpvalues - i - 1] = upvalue;
            }

            ObjCode *body = AS_CODE(pop_stack(vm));
            char *name = AS_CSTRING(pop_stack(vm));

            list_t *defaults = list_create(sizeof(Value));

            // Adjust the stack pointer to the first parameter
            vm->sp -= numParams;

            // Populate the parameter list directly from the stack
            for (int i = 0; i < numParams; i++)
            {
                Value param = vm->stack[vm->sp + i];
                list_add(defaults, &param);
            }

            Object *fun_obj = new_func(name, body, defaults, upvalues, NULL);

            // Push the new closure onto the stack
            push_stack(vm, NEW_OBJ(add_obj(vm, fun_obj)));

            break;
        }

        case OP_LOAD_UPVALUE:
        {
            int index = code[pc++];
            UpValue *upValue = function->upvalues[index];
            if (upValue->index != -1)
                push_stack(vm, vm->stack[upValue->index]);
            else
                push_stack(vm, upValue->value);
            break;
        }

        case OP_STORE_UPVALUE:
        {
            int index = code[pc++];
            UpValue *upValue = function->upvalues[index];
            if (upValue->index != -1)
                vm->stack[upValue->index] = pop_stack(vm);
            else
                function->upvalues[index]->value = pop_stack(vm);
            break;
        }

        case OP_PUSH_SLICE:
        {
            // Pop the slice values from the stack
            Value step = pop_stack(vm);
            Value end = pop_stack(vm);
            Value start = pop_stack(vm);

            if (!IS_NUM(start) || !IS_NUM(end))
                vm_error(vm, "Slice [start] and [end] must be numbers.");

            // Create a new slice object
            if (!IS_NIL(step) && !IS_NUM(step))
                vm_error(vm, "Slice [step] must be nil or a number.");
            else
            {
                Value sequence = pop_stack(vm);
                if (IS_SEQUENCE(sequence))
                {
                    double end_num = as_number(end);
                    Value slice = get_slice(AS_OBJ(sequence), as_number(start), as_number(end),
                                            IS_NIL(step) ? 1.0 : as_number(step));
                    push_stack(vm, slice); // Push the slice onto the stack
                }
                else
                    vm_error(vm, "Slice operand must be a list or string.");
            }

            break;
        }

        case OP_GET_ITEM:
        {
            Value index = pop_stack(vm);     // Get the index from the stack
            Value container = pop_stack(vm); // Get the container from the stack

            if (!IS_OBJ(container))
                vm_error(vm, "Unsupported operand type for get item operator.\n");

            switch (OBJ_TYPE(container))
            {
            case OBJ_LIST:
            {
                list_t *list = as_list(container);
                if (list->size == 0)
                    push_stack(vm, NEW_NIL());
                else
                {
                    int _index = as_number(index);
                    Value item = *(Value *)list_getAt(list, _index);
                    push_stack(vm, item); // Avoid unsafe memory access
                }
                break;
            }
            case OBJ_MAP:
            {
                table_t *table = AS_MAP(container)->table;

                // Value item = *(Value *)ht_get(table, as_string(index));
                Value item = map_get(AS_MAP(container), index);
                push_stack(vm, item); // Push NIL if key not found
                break;
            }

            case OBJ_STRING:
            {

                char *str = as_string(container);                      // Convert Value to char*
                int _index = get_index(as_number(index), strlen(str)); // Convert index to int

                // Convert the character to a string (newly allocated)
                char *_char = malloc(2); // 1 char + null terminator
                _char[0] = str[_index];
                _char[1] = '\0';
                push_stack(vm, NEW_OBJ(add_obj(vm, new_pistring(_char))));
                break;
            }

            default:
                vm_error(vm, "Unsupported operand type for get item operator.\n");
            }
            break;
        }

        case OP_SET_ITEM:
        {
            Value index = pop_stack(vm);     // The index/key
            Value container = pop_stack(vm); // The container (list/map)
            Value value = pop_stack(vm);     // The value to set

            if (!IS_OBJ(container))
                vm_error(vm, "Unsupported operand type for set item operator.\n");

            switch (OBJ_TYPE(container))
            {
            case OBJ_LIST:
            {
                list_t *list = as_list(container);
                int _index = get_index(as_number(index), list_size(list));

                list_set(list, _index, &value);
                break;
            }

            case OBJ_MAP:
            {
                table_t *table = AS_MAP(container)->table;

                map_set(AS_MAP(container), index, value);
                break;
            }

            case OBJ_STRING:
                vm_error(vm, "Cannot modify immutable string.\n");
                break;

            default:
                vm_error(vm, "Unsupported operand type for set item operator.\n");
            }
            break;
        }

        case OP_RETURN:
        {
            // Handle return operation
            Value retval = pop_stack(vm);

            for (int i = vm->sp - 1; i >= vm->bp; i--)
                remove_upvalue(vm, i);

            Frame *frame = pop_frame(vm);

            while (vm->iter_sp > frame->iters_top)
                vm->iter_sp--;

            if (vm->iter_sp != -1)
                iter = vm->iters[vm->iter_sp];

            vm->pc = frame->pc;
            vm->bp = frame->bp;
            vm->sp = frame->sp;
            vm->ip = frame->ip;

            vm->code = frame->code;

            free_frame(frame);

            push_stack(vm, retval);

            return;
        }

        case OP_HALT:
        {
            vm->running = false;
            // Halt the VM
            return;
        }

        case OP_NO:
            break;

        case OP_PUSH_NIL:
            push_stack(vm, NEW_NIL());
            break;

        case OP_DEBUG:
            // Handle debug operation
            printf("[DEBUG] Current PC: %d\n", pc);
            break;

        // Add more cases for other opcodes as needed
        default:
            vm_errorf(vm, "Unknown opcode: [%d]\n", op);

            vm->pc = pc;
        }

#ifdef __EMSCRIPTEN__
// For Emscripten, use a much larger GC threshold or manual GC
#define EMSCRIPTEN_GC_THRESHOLD (1024 * 1024 * 10) // 10MB worth of objects
        if (vm->counter >= EMSCRIPTEN_GC_THRESHOLD)
        {
            run_gc(vm);
            vm->counter = 0;
        }
#else
        if (vm->counter >= vm->next_gc)
        {




#ifdef DEBUG
            printf("[DEBUG] SP: %d\n", vm->sp);
            printf("[GC] Running garbage collection...\n");
            printf("[GC] Before: %d objects in memory\n", count_objs(vm));
            run_gc(vm);
            printf("[GC] After: %d objects in memory\n", count_objs(vm));
            printf("[GC] Counter: %d\n", vm->counter);
#endif
            int before = count_objs(vm);
            run_gc(vm);
            int count = before - vm->obj_count;

            vm->counter = 0;

            // Adjust next threshold adaptively
            if (count > 128)
                vm->next_gc /= 2; // GC did not help, try more often
            else
                vm->next_gc *= 2; // GC was effective, increase threshold
            vm->obj_count = before;

            // Clamp bounds (prevent very frequent or very rare GC)
            if (vm->next_gc < 1024)
                vm->next_gc = 1024;
            else if (vm->next_gc > 1024 * 1024)
                vm->next_gc = 1024 * 1024;
        }
#endif
        vm->pc = pc;
    }
}

/**
 * Frees the memory allocated for a virtual machine instance.
 *
 * This function is used to clean up the memory allocated to the virtual
 * machine structure. It first frees the memory allocated to the global
 * hash table and then frees the virtual machine structure itself.
 *
 * @param vm The virtual machine instance to be deallocated.
 */
void free_vm(vm_t *vm)
{
    audio_stopAll();

    if (vm->cart)
    {
        cart_free(vm->cart);
    }
    // Free the memory allocated for the global hash table
    ht_free(vm->globals);

    // Free the memory allocated for the mutex
    pthread_mutex_destroy(&vm->lock);

    // Free the virtual machine structure itself
    free(vm);
}
