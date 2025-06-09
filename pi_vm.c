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

    // Initialize global constants

    // Math constants
    ht_put(vm->globals, "PI", &NEW_NUM(PI));
    ht_put(vm->globals, "E", &NEW_NUM(E));

    // Graphics constants
    ht_put(vm->globals, "WIDTH", &NEW_NUM(SCREEN_WIDTH));
    ht_put(vm->globals, "HEIGHT", &NEW_NUM(SCREEN_HEIGHT));

    // Add native functions

    // Math functions
    ht_put(vm->globals, "floor", new_native("floor", pi_floor));
    ht_put(vm->globals, "ceil", new_native("ceil", pi_ceil));
    ht_put(vm->globals, "round", new_native("round", pi_round));
    ht_put(vm->globals, "rand", new_native("rand", pi_rand));
    ht_put(vm->globals, "rand_n", new_native("rand_n", pi_rand_n));
    ht_put(vm->globals, "rand_i", new_native("rand_i", pi_rand_i));
    ht_put(vm->globals, "sqrt", new_native("sqrt", pi_sqrt));
    ht_put(vm->globals, "sin", new_native("sin", pi_sin));
    ht_put(vm->globals, "cos", new_native("cos", pi_cos));
    ht_put(vm->globals, "tan", new_native("tan", pi_tan));
    ht_put(vm->globals, "asin", new_native("asin", pi_asin));
    ht_put(vm->globals, "acos", new_native("acos", pi_acos));
    ht_put(vm->globals, "atan", new_native("atan", pi_atan));
    ht_put(vm->globals, "deg", new_native("deg", pi_deg));
    ht_put(vm->globals, "rad", new_native("rad", pi_rad));
    ht_put(vm->globals, "sum", new_native("sum", pi_sum));
    ht_put(vm->globals, "exp", new_native("exp", pi_exp));
    ht_put(vm->globals, "log2", new_native("log2", pi_log2));
    ht_put(vm->globals, "log10", new_native("log10", pi_log10));
    ht_put(vm->globals, "pow", new_native("pow", pi_pow));
    ht_put(vm->globals, "abs", new_native("abs", pi_abs));
    ht_put(vm->globals, "mean", new_native("mean", pi_mean));
    ht_put(vm->globals, "avg", new_native("avg", pi_avg));
    ht_put(vm->globals, "var", new_native("var", pi_var));
    ht_put(vm->globals, "dev", new_native("dev", pi_dev));
    ht_put(vm->globals, "median", new_native("median", pi_median));
    ht_put(vm->globals, "mode", new_native("mode", pi_mode));
    ht_put(vm->globals, "max", new_native("max", pi_max));
    ht_put(vm->globals, "min", new_native("min", pi_min));

    // Graphics functions
    ht_put(vm->globals, "pixel", new_native("pixel", pi_pixel));
    ht_put(vm->globals, "line", new_native("line", pi_line));
    ht_put(vm->globals, "draw", new_native("draw", pi_draw));
    ht_put(vm->globals, "clear", new_native("clear", pi_clear));
    ht_put(vm->globals, "circ", new_native("circ", pi_circ));
    ht_put(vm->globals, "rect", new_native("rect", pi_rect));
    ht_put(vm->globals, "poly", new_native("poly", pi_poly));
    ht_put(vm->globals, "sprite", new_native("sprite", pi_sprite));
    ht_put(vm->globals, "color", new_native("color", pi_color));

    // time functions
    ht_put(vm->globals, "sleep", new_native("sleep", pi_sleep));
    ht_put(vm->globals, "time", new_native("time", _pi_time));

    // IO functions
    ht_put(vm->globals, "println", new_native("println", pi_println));
    ht_put(vm->globals, "print", new_native("print", pi_print));
    ht_put(vm->globals, "printf", new_native("printf", pi_printf));
    ht_put(vm->globals, "key", new_native("key", pi_key));
    ht_put(vm->globals, "text", new_native("text", pi_text));
    ht_put(vm->globals, "input", new_native("input", pi_input));

    // String functions
    ht_put(vm->globals, "char", new_native("char", pi_char));
    ht_put(vm->globals, "ord", new_native("ord", pi_ord));
    ht_put(vm->globals, "trim", new_native("trim", pi_trim));
    ht_put(vm->globals, "upper", new_native("upper", pi_upper));
    ht_put(vm->globals, "lower", new_native("lower", pi_lower));
    ht_put(vm->globals, "replace", new_native("replace", pi_replace));
    ht_put(vm->globals, "is_upper", new_native("isUpper", pi_isUpper));
    ht_put(vm->globals, "is_lower", new_native("isLower", pi_isLower));
    ht_put(vm->globals, "is_digit", new_native("isDigit", pi_isDigit));
    ht_put(vm->globals, "is_numeric", new_native("isNumeric", pi_isNumeric));
    ht_put(vm->globals, "is_alpha", new_native("isAlpha", pi_isAlpha));
    ht_put(vm->globals, "is_alnum", new_native("isAlnum", pi_isAlnum));

    // Audio functions
    ht_put(vm->globals, "sound", new_native("sound", pi_sound));
    ht_put(vm->globals, "music", new_native("music", pi_music));

    // System functions
    ht_put(vm->globals, "fps", new_native("fps", pi_fps));
    ht_put(vm->globals, "error", new_native("error", pi_error));
    ht_put(vm->globals, "zen", new_native("zen", pi_zen));
    ht_put(vm->globals, "cursor", new_native("cursor", pi_cursor));
    ht_put(vm->globals, "mouse", new_native("mouse", pi_mouse));

    // type functions
    ht_put(vm->globals, "type", new_native("type", _pi_type));
    ht_put(vm->globals, "is_num", new_native("is_num", pi_isNum));
    ht_put(vm->globals, "is_str", new_native("is_str", pi_isStr));
    ht_put(vm->globals, "is_bool", new_native("is_bool", pi_isBool));
    ht_put(vm->globals, "is_list", new_native("is_list", pi_isList));
    ht_put(vm->globals, "is_map", new_native("is_map", pi_isMap));
    ht_put(vm->globals, "as_num", new_native("as_num", pi_asNum));
    ht_put(vm->globals, "as_str", new_native("as_str", pi_asStr));
    ht_put(vm->globals, "as_bool", new_native("as_bool", pi_asBool));

    // collection manipulation functions
    ht_put(vm->globals, "push", new_native("push", pi_push));
    ht_put(vm->globals, "pop", new_native("pop", pi_pop));
    ht_put(vm->globals, "peek", new_native("peek", pi_peek));
    ht_put(vm->globals, "empty", new_native("empty", pi_empty));
    ht_put(vm->globals, "sort", new_native("sort", pi_sort));
    ht_put(vm->globals, "insert", new_native("insert", pi_insert));
    ht_put(vm->globals, "unshift", new_native("unshift", pi_unshift));
    ht_put(vm->globals, "remove", new_native("remove", pi_remove));
    ht_put(vm->globals, "append", new_native("append", pi_append));
    ht_put(vm->globals, "contains", new_native("contains", pi_contains));
    ht_put(vm->globals, "index_of", new_native("index_of", pi_indexOf));
    ht_put(vm->globals, "reverse", new_native("reverse", pi_reverse));
    ht_put(vm->globals, "shuffle", new_native("shuffle", pi_shuffle));
    ht_put(vm->globals, "copy", new_native("copy", pi_copy));
    ht_put(vm->globals, "slice", new_native("slice", pi_slice));
    ht_put(vm->globals, "len", new_native("len", pi_len));

    // functional programming
    ht_put(vm->globals, "map", new_native("map", _pi_map));
    ht_put(vm->globals, "filter", new_native("filter", pi_filter));
    ht_put(vm->globals, "reduce", new_native("reduce", pi_reduce));
    ht_put(vm->globals, "find", new_native("find", pi_find));

    // matrix manipulation functions
    ht_put(vm->globals, "size", new_native("size", pi_size));

    ht_put(vm->globals, "mult", new_native("mult", pi_mult));
    ht_put(vm->globals, "dot", new_native("dot", pi_dot));
    ht_put(vm->globals, "cross", new_native("cross", pi_cross));

    ht_put(vm->globals, "eye", new_native("eye", pi_eye));
    ht_put(vm->globals, "zeros", new_native("zeros", pi_zeros));
    ht_put(vm->globals, "ones", new_native("ones", pi_ones));
    ht_put(vm->globals, "is_mat", new_native("is_mat", pi_isMat));

    // object manipulation functions
    ht_put(vm->globals, "clone", new_native("clone", pi_clone));
    ht_put(vm->globals, "values", new_native("values", pi_values));
    ht_put(vm->globals, "keys", new_native("keys", pi_keys));

    vm->iter_sp = -1;
    vm->frame_sp = 0;

    vm->screen = screen;

    vm->running = true;

    vm->fps = TARGET_FPS;

    pthread_mutex_init(&vm->lock, NULL);

    mark_constants(vm);

    vm->counter = 0;

    vm->openUpvalues = NULL;

    return vm;
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

    // printf("[DEBUG] Adding object at %p\n", (void *)obj);
    // Add the object to the front of the list
    obj->next = vm->objects;
    vm->objects = obj;

    // Return the newly added object
    return obj;
}

static inline int count_objs(vm_t *vm)
{
    int count = 0;
    Object *obj = vm->objects;
    while (obj)
    {
        printf("[DEBUG] Counting object at %p\n", (void *)obj);
        count++;
        obj = obj->next;
    }
    return count;
}

/**
 * Reports a virtual machine error with a specified message.
 *
 * This function outputs an error message to the standard error stream,
 * indicating a critical error in the virtual machine operation. The
 * program will terminate immediately after displaying the error message.
 *
 * @param message The error message to be displayed.
 */

static void vm_error(vm_t *vm, const char *message)
{

    for (int i = 0; i < list_size(vm->instrs); i++)
    {
        instr_t *instr = list_getAt(vm->instrs, i);
        if (instr->offset == vm->pc)
        {
            fprintf(stderr, "\n\033[1;31m[RUNTIME ERROR]\033[0m: %s\n", message);
            fprintf(stderr, "\033[90mAt line %d, column %d\033[0m\n",
                    instr->line, instr->column - 1);
            exit(EXIT_FAILURE);
        }
    }

    // instr_t *instr = list_getAt(vm->instrs, vm->pc); // Get the instruction at the current PC

    // fprintf(stderr, "\n\033[1;31m[RUNTIME ERROR]\033[0m: %s\n", message);
    // fprintf(stderr, "\033[90mAt line %d, column %d\033[0m\n",
    //         instr->line, instr->column - 1);

    // exit(EXIT_FAILURE);
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

void push_frame(vm_t *vm, Frame *frame)
{
    if (vm->frame_sp >= STACK_MAX)
        vm_error(vm, "Stack overflow: Attempted to push onto a full stack");

    vm->frames[vm->frame_sp++] = frame;
}

Frame *pop_frame(vm_t *vm)
{
    if (vm->frame_sp <= 0)
        vm_error(vm, "Stack underflow: Attempted to pop from an empty stack");

    return vm->frames[--vm->frame_sp];
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

static Value bind(Function *function, Object *instance)
{
    Object *fn = new_func(function->name, function->body,
                          function->params, NULL, instance);
    ((Function *)fn)->is_method = true;
    return NEW_OBJ(fn);
}

static Object *construct(vm_t *vm, PiMap *map, size_t argc, Value *argv)
{

    table_t *table = ht_create(sizeof(Value));
    list_t *keys = ht_keys(map->table);

    Object *instance = new_map(table, true);
    ((PiMap *)instance)->proto = map;

    for (size_t i = 0; i < keys->size; i++)
    {
        char *key = string_get(keys, i);
        if (strcmp(key, "constructor") != 0)
        {
            Value value = *(Value *)ht_get(map->table, key);
            if (IS_FUN(value))
            {
                Value fn = bind(AS_FUN(value), instance);
                ht_put(table, key, &fn);
            }
            else
                ht_put(table, key, ht_get(map->table, key));
        }
    }

    vm->stack[vm->sp] = NEW_OBJ(instance);

    // Prepare arguments with 'this' as the first argument
    Value *fargs = (Value *)malloc(sizeof(Value) * (argc + 1));
    fargs[0] = NEW_OBJ(instance); // 'this' reference
    memcpy(fargs + 1, argv, sizeof(Value) * argc);

    // Invoke the constructor if it exists
    void *item = ht_get(map->table, "constructor");

    Value constructor = item ? *(Value *)item : NEW_NIL();

    if (IS_FUN(constructor))
    {
        AS_FUN(constructor)->is_method = false;
        instance = AS_OBJ(call_func(vm, AS_FUN(constructor), argc + 1, fargs));
    }

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

        // Cast the opcode to the OpCode enum
        switch ((OpCode)op)
        {
        case OP_LOAD_CONST:
        {
            // Read a two-byte short value from the bytecode to get the constant index
            // index = (code[pc++] << 8) | code[pc++];
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
            char *name = read_name(vm, index);
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
                fprintf(stderr, "Unknown opcode: [%d]\n", op);
                exit(EXIT_FAILURE);
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
                    {
                        fprintf(stderr, "Memory allocation failed.\n");
                        exit(EXIT_FAILURE);
                    }

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
                // if (IS_LIST(left))
                // {
                //     PiList *list = AS_LIST(left);
                //     list_add(list->items, &right);
                //     push_stack(vm, left);
                //     break;
                // }
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
                    {
                        fprintf(stderr, "Unsupported operand types for binary operator [*].\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    fprintf(stderr, "Unsupported operand types for binary operator [*].\n");
                    exit(EXIT_FAILURE);
                }
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
                    {
                        is_matrix = false;
                    }
                }
                else
                {
                    is_matrix = false;
                }
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

            // case OP_PUSH_LIST:
            // {
            //     bool is_numeric = true;
            //     // Read the number of elements in the list
            //     int numElements = code[pc++] << 8;
            //     numElements |= code[pc++];

            //     // Create a new list
            //     list_t *list = list_create(sizeof(Value));

            //     // Adjust the stack pointer to the first element of the list
            //     vm->sp -= numElements;

            //     // Populate the list directly from the stack
            //     for (int i = 0; i < numElements; i++)
            //     {
            //         Value element = vm->stack[vm->sp + i];
            //         if (is_numeric && !IS_NUM(element))
            //             is_numeric = false;
            //         list_add(list, &element);
            //     }

            //     // Push the new list onto the stack
            //     Object *l_obj = add_obj(vm, new_list(list));
            //     ((PiList *)l_obj)->is_numeric = is_numeric;
            //     push_stack(vm, NEW_OBJ(l_obj));
            //     break;
            // }

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

                if (IS_FUN(value))
                    AS_FUN(value)->is_method = true;

                char *key = AS_CSTRING(vm->stack[i + 1]);
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

            PiCode *body = AS_CODE(pop_stack(vm));
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
            Object *function = new_func(name, body->data, defaults, NULL, NULL);

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

            PiCode *body = AS_CODE(pop_stack(vm));
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
            Object *fun_obj = new_func(name, body->data, defaults, upvalues, NULL);
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

            free(frame);

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
        {
            printf("Unknown opcode: [%d]\n", op);
            exit(EXIT_FAILURE);
        }
            vm->pc = pc;
        }

        if (vm->counter >= NEXT_GC)
        {
#ifdef DEBUG
            printf("[DEBUG] SP: %d\n", vm->sp);
            printf("[GC] Running garbage collection...\n");
            printf("[GC] Before: %d objects in memory\n", count_objs(vm));
            run_gc(vm);
            printf("[GC] After: %d objects in memory\n", count_objs(vm));
#endif
            run_gc(vm);
            vm->counter = 0;
        }
        vm->pc = pc;
    }
}

void free_vm(vm_t *vm)
{

    ht_free(vm->globals);

    pthread_mutex_destroy(&vm->lock);
    free(vm);
}