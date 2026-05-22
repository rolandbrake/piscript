#include <math.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "pi_vm.h"

#include "pi_opcode.h"
#include "pi_value.h"

#include "pi_string.h"
#include "common.h"
#include "pi_func.h"
#include "gc.h"

#include "builtin/pi_builtin.h"

#define GC_MIN_THRESHOLD 4096
#define GC_MAX_THRESHOLD (1024 * 1024 * 8)
#define REGISTER_COUNT 256

// Register file structure
typedef struct {
    Value regs[REGISTER_COUNT];
    uint8_t used_regs;
} RegisterFile;

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
 * setting initial values for the program counter, register file,
 * base pointer, and other components.
 */
vm_t *init_vm(compiler_t *comp, Screen *screen)
{
    // Allocate memory for the virtual machine instance
    vm_t *vm = (vm_t *)malloc(sizeof(vm_t));

    // Initialize program counter, register file, and base pointer
    vm->pc = 0;
    vm->bp = 0;
    vm->ip = 0;
    
    // Initialize register file
    RegisterFile *regfile = (RegisterFile *)malloc(sizeof(RegisterFile));
    memset(regfile, 0, sizeof(RegisterFile));
    for (int i = 0; i < REGISTER_COUNT; i++) {
        regfile->regs[i] = NEW_NIL();
    }
    regfile->used_regs = 0;
    vm->regfile = regfile;

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

    vm->frameInterval_ms = 1000 / TARGET_FPS;
    vm->last_drawTicks = 0;

    return vm;
}

/**
 * Resets an existing virtual machine to run new code.
 */
void vm_reset(vm_t *vm, compiler_t *comp)
{
    // Reset program counter, register file, and base pointer
    vm->pc = 0;
    vm->bp = 0;
    vm->ip = 0;
    
    // Clear register file
    for (int i = 0; i < REGISTER_COUNT; i++) {
        vm->regfile->regs[i] = NEW_NIL();
    }
    vm->regfile->used_regs = 0;

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

    vm->frameInterval_ms = 1000 / TARGET_FPS;
    vm->last_drawTicks = 0;

    // Mark new constants from the new compiler for GC
    mark_constants(vm);
}

/**
 * Adds an object to the VM's object list.
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
    vm->counter++; // Track new allocations

    return obj;
}

/**
 * Register access functions
 */
static inline void set_register(vm_t *vm, uint8_t reg, Value value)
{
    if (reg >= REGISTER_COUNT)
        vm_error(vm, "Register index out of bounds");
    vm->regfile->regs[reg] = value;
    if (reg >= vm->regfile->used_regs)
        vm->regfile->used_regs = reg + 1;
}

static inline Value get_register(vm_t *vm, uint8_t reg)
{
    if (reg >= REGISTER_COUNT)
        vm_error(vm, "Register index out of bounds");
    return vm->regfile->regs[reg];
}

static inline void copy_register(vm_t *vm, uint8_t dest, uint8_t src)
{
    set_register(vm, dest, get_register(vm, src));
}

/**
 * Counts the number of objects in the virtual machine's object list.
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
 */
void vm_errorf(vm_t *vm, const char *fmt, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    vm_error(vm, buffer);
}

/**
 * Pushes a frame onto the stack.
 */
void push_frame(vm_t *vm, Frame *frame)
{
    if (vm->frame_sp >= STACK_MAX)
        vm_error(vm, "Stack overflow: Attempted to push onto a full stack");

    vm->frames[vm->frame_sp++] = frame;
}

/**
 * Pops a frame from the stack.
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
 */
static inline char *read_name(vm_t *vm, int index)
{
    return string_get(vm->names, index);
}

/**
 * Checks if the given value is considered false.
 */
static inline bool is_false(vm_t *vm, Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static inline int read_short(vm_t *vm)
{
    uint8_t *code = (uint8_t *)vm->code->data;
    int high = code[vm->pc++] & 0xFF;
    int low = code[vm->pc++] & 0xFF;
    return (high << 8) | low;
}

static inline int _read_short(uint8_t *code, int pc)
{
    int high = code[pc] & 0xFF;
    int low = code[pc + 1] & 0xFF;
    return (high << 8) | low;
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
    _upvalue->value = vm->regfile->regs[index];
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
        upvalue->value = vm->regfile->regs[index];

        if (prev == NULL)
            vm->openUpvalues = upvalue->next;
        else
            prev->next = upvalue->next;
    }
}

/**
 * Bind a function to an instance.
 */
static Value bind(vm_t *vm, Function *function, Object *instance)
{
    Object *fn = new_func(function->name, function->body,
                          function->params, NULL, instance);

    ((Function *)fn)->is_method = true;
    add_obj(vm, fn);

    return NEW_OBJ(fn);
}

/**
 * Constructs a new object instance from a given prototype map.
 */
static Object *construct(vm_t *vm, PiMap *map, size_t argc, Value *argv)
{
    table_t *table = ht_create(sizeof(Value));
    char **keys = ht_keys(map->table);
    int size = ht_length(map->table);

    Object *instance = new_map(table, true);
    ((PiMap *)instance)->proto = map;

    for (size_t i = 0; i < size; i++)
    {
        char *key = keys[i];
        if (strcmp(key, "constructor") != 0)
        {
            Value value = *(Value *)ht_get(map->table, key);
            if (IS_FUN(value))
            {
                Value fn = bind(vm, AS_FUN(value), instance);
                ht_put(table, key, &fn);
            }
            else
                ht_put(table, key, ht_get(map->table, key));
        }
    }

    Value *fargs = (Value *)malloc(sizeof(Value) * (argc + 1));
    fargs[0] = NEW_OBJ(instance);
    memcpy(fargs + 1, argv, sizeof(Value) * argc);

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
    uint8_t dest_reg, src_reg1, src_reg2;

    uint8_t *code = (uint8_t *)vm->code->data;

    Value value;
    Value nilValue;

    Object *iter = NULL;

    UpValue *upValue;

    Function *function = (Function *)vm->function;

    while (pc < length && vm->running)
    {
        op = code[pc++];
        vm->ip++;

        switch ((OpCode)op)
        {
        case OP_LOAD_CONST:
        {
            index = (code[pc++] << 8);
            index |= code[pc++];
            dest_reg = code[pc++];
            
            Value constant = *(Value *)list_getAt(vm->constants, index);
            set_register(vm, dest_reg, constant);
            break;
        }

        case OP_STORE_GLOBAL:
        {
            index = code[pc++];
            src_reg = code[pc++];
            char *name = read_name(vm, index);

            Value _newValue = get_register(vm, src_reg);
            ht_put(vm->globals, name, &_newValue);
            break;
        }

        case OP_LOAD_GLOBAL:
        {
            index = code[pc++];
            dest_reg = code[pc++];
            char *name = string_get(vm->names, index);
            Value *_value = ht_get(vm->globals, name);
            if (_value == NULL)
            {
                nilValue = NEW_NIL();
                _value = &nilValue;
            }
            set_register(vm, dest_reg, *_value);
            break;
        }

        case OP_LOAD_LOCAL:
        {
            uint8_t local_index = code[pc++];
            dest_reg = code[pc++];
            Value value = vm->regfile->regs[vm->bp + local_index];
            set_register(vm, dest_reg, value);
            break;
        }

        case OP_STORE_LOCAL:
        {
            uint8_t local_index = code[pc++];
            src_reg = code[pc++];
            vm->regfile->regs[vm->bp + local_index] = get_register(vm, src_reg);
            break;
        }

        case OP_MOVE:
        {
            dest_reg = code[pc++];
            src_reg = code[pc++];
            copy_register(vm, dest_reg, src_reg);
            break;
        }

        case OP_JUMP_IF_FALSE:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]);
            src_reg = code[pc + 2];

            Value value = get_register(vm, src_reg);
            if (!as_bool(value))
                pc += offset - 1;
            else
                pc += 3;
            break;
        }

        case OP_JUMP:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]);
            pc += offset - 1;
            break;
        }

        case OP_JUMP_IF_TRUE:
        {
            int offset = (int16_t)((code[pc] << 8) | code[pc + 1]);
            src_reg = code[pc + 2];

            Value value = get_register(vm, src_reg);
            if (as_bool(value))
                pc += offset - 1;
            else
                pc += 3;
            break;
        }

        case OP_COMPARE:
        {
            uint8_t compare_op = code[pc++];
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            dest_reg = code[pc++];

            Value left = get_register(vm, src_reg1);
            Value right = get_register(vm, src_reg2);

            bool result = false;
            int cmp = compare(left, right);

            switch (compare_op)
            {
            case 0: result = (cmp == 0); break;
            case 1: result = (cmp != 0); break;
            case 2: result = (cmp > 0); break;
            case 3: result = (cmp < 0); break;
            case 4: result = (cmp >= 0); break;
            case 5: result = (cmp <= 0); break;
            default: vm_errorf(vm, "Unknown opcode: [%d]", compare_op);
            }
            set_register(vm, dest_reg, NEW_BOOL(result));
            break;
        }

        case OP_BINARY:
        {
            uint8_t binary_op = code[pc++];
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            dest_reg = code[pc++];

            Value left = get_register(vm, src_reg1);
            Value right = get_register(vm, src_reg2);

            switch (binary_op)
            {
            case 0: // "+"
            {
                if (is_numeric(left) && is_numeric(right))
                {
                    set_register(vm, dest_reg, NEW_NUM(as_number(left) + as_number(right)));
                    break;
                }

                if (IS_STRING(left) || IS_STRING(right))
                {
                    char *l_str = as_string(left);
                    char *r_str = as_string(right);

                    size_t len = strlen(l_str) + strlen(r_str) + 1;
                    char *res = (char *)malloc(len);
                    if (!res)
                        vm_error(vm, "Memory allocation failed.");

                    strcpy(res, l_str);
                    strcat(res, r_str);

                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_pistring(res))));

                    free(l_str);
                    free(r_str);
                    break;
                }

                if (IS_LIST(left))
                {
                    PiList *list = AS_LIST(left);
                    list_add(list->items, &right);

                    if (list->rows == 1 && list->cols >= 0)
                    {
                        if (!IS_NUM(right))
                        {
                            list->rows = -1;
                            list->cols = -1;
                            list->is_numeric = false;
                        }
                        else
                            list->cols++;
                    }
                    else if (list->rows > 1 && list->cols > 0)
                    {
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
                                list->rows++;
                        }
                    }
                    else
                    {
                        if (list->items->size == 1 && IS_NUM(right) && IS_NUM(((Value *)list->items->data)[0]))
                        {
                            list->is_numeric = true;
                            list->rows = 1;
                            list->cols = 2;
                        }
                    }

                    set_register(vm, dest_reg, left);
                    break;
                }
                if (IS_NAN(left) || IS_NAN(right))
                {
                    set_register(vm, dest_reg, NEW_NUM(NAN));
                    break;
                }
                vm_error(vm, "Unsupported operand types for binary operator [+].");
            }
            case 1: // "-"
            {
                if (is_numeric(left) && is_numeric(right))
                {
                    set_register(vm, dest_reg, NEW_NUM(as_number(left) - as_number(right)));
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
                        set_register(vm, dest_reg, left);
                        break;
                    }

                    if (IS_STRING(left))
                    {
                        char *l_str = as_string(left);
                        char *r_str = as_string(right);

                        size_t l_len = strlen(l_str);
                        size_t r_len = strlen(r_str);

                        char *res = (char *)malloc(l_len + 1);
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

                        strcpy(w_ptr, r_ptr);

                        set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_pistring(res))));

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
                    set_register(vm, dest_reg, NEW_NUM(as_number(left) * as_number(right)));
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
                                    Value *a_val = (Value *)list_getAt(rowA, k);
                                    double a = as_number(*a_val);

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
                        set_register(vm, dest_reg, NEW_OBJ(res_obj));
                        break;
                    }
                    else if (IS_LIST(left))
                    {
                        int count = (int)as_number(right);
                        list_t *list = as_list(left);

                        list_t *result = list_create(list->i_size);
                        for (int i = 0; i < count; i++)
                            list_addAll(result, list);

                        Object *_result = new_list(result);
                        if (AS_LIST(left)->is_numeric)
                            ((PiList *)_result)->is_numeric = true;

                        set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, _result)));
                    }
                    else if (IS_STRING(left))
                    {
                        int count = (int)as_number(right);
                        char *str = as_string(left);
                        size_t o_len = strlen(str);
                        size_t r_len = o_len * count;

                        char *result = (char *)malloc(r_len + 1);
                        result[0] = '\0';

                        for (int i = 0; i < count; i++)
                            strcat(result, str);

                        set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_pistring(result))));
                        free(str);
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
                    set_register(vm, dest_reg, NEW_NUM(INFINITY));
                    break;
                }

                double numerator = as_number(left);
                set_register(vm, dest_reg, NEW_NUM(numerator / denominator));
                break;
            }
            case 4: // "%"
            {
                double denominator = as_number(right);

                if ((int)denominator == 0)
                    set_register(vm, dest_reg, NEW_NAN());
                else
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) % (int)denominator));
                break;
            }
            case 5: // "&&"
                set_register(vm, dest_reg, NEW_BOOL(as_bool(left) && as_bool(right)));
                break;
            case 6: // "||"
                set_register(vm, dest_reg, NEW_BOOL(as_bool(left) || as_bool(right)));
                break;
            case 7: // "**"
                set_register(vm, dest_reg, NEW_NUM(pow(as_number(left), as_number(right))));
                break;
            case 8: // "&"
            {
                if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) & (int)as_number(right)));
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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [&].");

                break;
            }

            case 9: // "|"
            {
                if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) | (int)as_number(right)));
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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
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

                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(res))));
                    break;
                }
                else if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) ^ (int)as_number(right)));

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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [^].");

                break;
            }

            case 11: // "<<"
            {
                if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) << (int)as_number(right)));

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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [<<].");

                break;
            }

            case 12: // ">>"
            {
                if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((int)as_number(left) >> (int)as_number(right)));

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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
                }
                else
                    vm_error(vm, "Unsupported operand types for binary operator [>>].");

                break;
            }

            case 13: // ">>>"
            {
                if (is_numeric(left))
                    set_register(vm, dest_reg, NEW_NUM((uint32_t)as_number(left) >> (uint32_t)as_number(right)));

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
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_list(result))));
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
                    set_register(vm, dest_reg, NEW_NUM(result));
                    break;
                }
                vm_error(vm, "Unsupported operand types for binary operator [.]");
            }

            case 15: // instance of operator [is]
            {
                if (!IS_MAP(left) || !IS_MAP(right))
                {
                    set_register(vm, dest_reg, NEW_BOOL(false));
                    break;
                }

                Object *inst_obj = AS_OBJ(left);
                Object *proto_obj = AS_OBJ(right);

                if (inst_obj->type != OBJ_MAP || proto_obj->type != OBJ_MAP)
                {
                    set_register(vm, dest_reg, NEW_BOOL(false));
                    break;
                }

                PiMap *map = (PiMap *)inst_obj;
                PiMap *proto = (PiMap *)proto_obj;

                while (map != NULL)
                {
                    if (map == proto)
                    {
                        set_register(vm, dest_reg, NEW_BOOL(true));
                        break;
                    }
                    map = map->proto;
                }

                if (!map)
                    set_register(vm, dest_reg, NEW_BOOL(false));

                break;
            }
            }
            break;
        }

        case OP_UNARY:
        {
            uint8_t unary_op = code[pc++];
            src_reg1 = code[pc++];
            dest_reg = code[pc++];
            
            Value operand = get_register(vm, src_reg1);

            switch (unary_op)
            {
            case 0: // Unary plus
                set_register(vm, dest_reg, NEW_NUM(as_number(operand)));
                break;

            case 1: // Unary minus
                set_register(vm, dest_reg, NEW_NUM(-as_number(operand)));
                break;

            case 2: // Logical NOT
                set_register(vm, dest_reg, NEW_BOOL(!as_bool(operand)));
                break;

            case 3: // Bitwise NOT
                set_register(vm, dest_reg, NEW_NUM(~(int)as_number(operand)));
                break;

            case 4: // Collection size
            {
                if (IS_COLLECTION(operand))
                {
                    switch (OBJ_TYPE(operand))
                    {
                    case OBJ_LIST:
                        set_register(vm, dest_reg, NEW_NUM(list_size(AS_LIST(operand)->items)));
                        break;
                    case OBJ_STRING:
                        set_register(vm, dest_reg, NEW_NUM(AS_STRING(operand)->length));
                        break;
                    case OBJ_MAP:
                        set_register(vm, dest_reg, NEW_NUM(map_size(AS_MAP(operand))));
                        break;
                    }
                }
                else
                    vm_error(vm, "Unsupported operand type for '#' operator.");

                break;
            }
            case 5: // "++"
                set_register(vm, dest_reg, NEW_NUM(as_number(operand) + 1));
                break;

            case 6: // "--"
                set_register(vm, dest_reg, NEW_NUM(as_number(operand) - 1));
                break;

            default:
                vm_error(vm, "Unknown unary operator.");
            }
            break;
        }

        case OP_CALL_FUNCTION:
        {
            uint8_t num_args = code[pc++];
            uint8_t func_reg = code[pc++];
            uint8_t result_reg = code[pc++];

            Value args[num_args];

            for (int i = num_args - 1; i >= 0; i--)
            {
                uint8_t arg_reg = code[pc++];
                args[i] = get_register(vm, arg_reg);
            }

            Value callee = get_register(vm, func_reg);

            if (IS_FUN(callee))
            {
                vm->function = AS_OBJ(callee);
                vm->pc = pc;
                Value result = call_func(vm, AS_FUN(callee), num_args, args);
                if (IS_OBJ(result))
                    add_obj(vm, AS_OBJ(result));
                set_register(vm, result_reg, result);
            }
            else if (IS_MAP(callee))
            {
                PiMap *map = AS_MAP(callee);
                if (map->is_instance)
                    vm_error(vm, "Attempt to call an Object instance.");
                else
                    set_register(vm, result_reg, NEW_OBJ(add_obj(vm, construct(vm, map, num_args, args))));
            }
            else
                vm_error(vm, "Attempt to call a non-function object.");
            break;
        }

        case OP_PUSH_ITER:
        {
            src_reg1 = code[pc++];
            Value iterable = get_register(vm, src_reg1);

            if (!IS_OBJ(iterable) || !is_iterable(AS_OBJ(iterable)))
                vm_error(vm, "Error: Object is not iterable.");

            iter = AS_OBJ(iterable);
            iter_reset(iter);
            vm->iters[++vm->iter_sp] = iter;
            break;
        }

        case OP_LOOP:
        {
            uint16_t address = (code[pc] << 8);
            address |= code[pc + 1];
            dest_reg = code[pc + 2];

            if (vm->iter_sp == -1)
                vm_error(vm, "Error: No active iterator.");

            iter = vm->iters[vm->iter_sp];

            if (iter_hasNext(iter))
            {
                if (iter->type == OBJ_MAP)
                {
                    PiMap *map = (PiMap *)iter;
                    ht_next(&map->it);
                    char *key = map->it.key;
                    set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_pistring(key))));
                }
                else
                {
                    Value value = iter_next(iter);
                    set_register(vm, dest_reg, value);
                }
                pc += 3;
            }
            else
            {
                vm->iter_sp--;
                pc += address - 1;
            }
            break;
        }

        case OP_POP_ITER:
        {
            if (vm->iter_sp != -1)
                iter = vm->iters[vm->iter_sp--];
            break;
        }

        case OP_PUSH_RANGE:
        {
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            src_reg3 = code[pc++];
            dest_reg = code[pc++];

            Value start = get_register(vm, src_reg1);
            Value end = get_register(vm, src_reg2);
            Value step = get_register(vm, src_reg3);

            if (!IS_NUM(start) || !IS_NUM(end))
                vm_error(vm, "PiRange `start` and `end` must be numbers.");

            if (!IS_NIL(step) && !IS_NUM(step))
                vm_error(vm, "PiRange `step` must be nil or a number.");
            else
            {
                double _start = as_number(start);
                double _end = as_number(end);
                double _step;
                if (IS_NIL(step))
                    _step = (_start < _end) ? 1.0 : -1.0;
                else
                    _step = as_number(step);
                Object *range = add_obj(vm, new_range(_start, _end, _step));
                set_register(vm, dest_reg, NEW_OBJ(range));
            }
            break;
        }

        case OP_PUSH_LIST:
        {
            int numElements = (code[pc++] << 8) | code[pc++];
            dest_reg = code[pc++];
            list_t *list = list_create(sizeof(Value));

            if (numElements == 0)
            {
                Object *l_obj = add_obj(vm, new_list(list));
                PiList *plist = (PiList *)l_obj;
                plist->is_numeric = true;
                plist->is_matrix = false;
                plist->rows = 0;
                plist->cols = 0;
                set_register(vm, dest_reg, NEW_OBJ(l_obj));
                break;
            }

            bool is_numeric = true;
            bool is_matrix = true;
            int rows = -1, cols = -1;

            for (int i = 0; i < numElements; i++)
            {
                uint8_t reg = code[pc++];
                Value v = get_register(vm, reg);
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
                uint8_t first_reg = code[pc - numElements];
                Value first = get_register(vm, first_reg);
                if (IS_LIST(first))
                {
                    PiList *pl0 = (PiList *)AS_OBJ(first);
                    if (pl0->is_numeric)
                    {
                        cols = pl0->items->size;
                        rows = numElements;
                        for (int i = 0; i < numElements; i++)
                        {
                            uint8_t reg = code[pc - numElements + i];
                            Value v = get_register(vm, reg);
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

            set_register(vm, dest_reg, NEW_OBJ(l_obj));
            break;
        }

        case OP_PUSH_MAP:
        {
            int numElements = code[pc++] << 8;
            numElements |= code[pc++];
            dest_reg = code[pc++];
            
            table_t *table = ht_create(sizeof(Value));

            for (int i = 0; i < numElements; i++)
            {
                uint8_t key_reg = code[pc++];
                uint8_t value_reg = code[pc++];
                
                Value key = get_register(vm, key_reg);
                Value value = get_register(vm, value_reg);
                
                char *key_str = AS_CSTRING(key);
                if (IS_FUN(value))
                    AS_FUN(value)->is_method = true;

                ht_put(table, key_str, &value);
            }

            Object *map = add_obj(vm, new_map(table, false));
            set_register(vm, dest_reg, NEW_OBJ(map));
            break;
        }

        case OP_PUSH_FUNCTION:
        {
            int numParams = code[pc++];
            uint8_t name_reg = code[pc++];
            uint8_t body_reg = code[pc++];
            dest_reg = code[pc++];

            ObjCode *body = AS_CODE(get_register(vm, body_reg));
            char *name = AS_CSTRING(get_register(vm, name_reg));

            list_t *defaults = list_create(sizeof(Value));

            for (int i = 0; i < numParams; i++)
            {
                uint8_t param_reg = code[pc++];
                Value param = get_register(vm, param_reg);
                list_add(defaults, &param);
            }

            Object *function = new_func(name, body, defaults, NULL, NULL);
            set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, function)));
            break;
        }

        case OP_PUSH_CLOSURE:
        {
            int numParams = code[pc++];
            int numUpvalues = code[pc++];
            uint8_t name_reg = code[pc++];
            uint8_t body_reg = code[pc++];
            dest_reg = code[pc++];

            UpValue **upvalues = ALLOCATE(UpValue *, numUpvalues + 1);

            for (int i = 0; i < numUpvalues; i++)
            {
                uint8_t is_local_reg = code[pc++];
                uint8_t index_reg = code[pc++];
                
                bool is_local = as_bool(get_register(vm, is_local_reg));
                int index = as_number(get_register(vm, index_reg));
                UpValue *upvalue;

                if (is_local)
                    upvalue = capture_upvalue(vm, vm->bp + index);
                else
                    upvalue = function->upvalues[index];

                upvalues[numUpvalues - i - 1] = upvalue;
            }
            upvalues[numUpvalues] = NULL;

            ObjCode *body = AS_CODE(get_register(vm, body_reg));
            char *name = AS_CSTRING(get_register(vm, name_reg));

            list_t *defaults = list_create(sizeof(Value));

            for (int i = 0; i < numParams; i++)
            {
                uint8_t param_reg = code[pc++];
                Value param = get_register(vm, param_reg);
                list_add(defaults, &param);
            }

            Object *fun_obj = new_func(name, body, defaults, upvalues, NULL);
            set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, fun_obj)));
            break;
        }

        case OP_LOAD_UPVALUE:
        {
            int upvalue_index = code[pc++];
            dest_reg = code[pc++];
            
            UpValue *upValue = function->upvalues[upvalue_index];
            if (upValue->index != -1)
                set_register(vm, dest_reg, vm->regfile->regs[upValue->index]);
            else
                set_register(vm, dest_reg, upValue->value);
            break;
        }

        case OP_STORE_UPVALUE:
        {
            int upvalue_index = code[pc++];
            src_reg1 = code[pc++];
            
            UpValue *upValue = function->upvalues[upvalue_index];
            if (upValue->index != -1)
                vm->regfile->regs[upValue->index] = get_register(vm, src_reg1);
            else
                function->upvalues[upvalue_index]->value = get_register(vm, src_reg1);
            break;
        }

        case OP_PUSH_SLICE:
        {
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            src_reg3 = code[pc++];
            src_reg4 = code[pc++];
            dest_reg = code[pc++];

            Value sequence = get_register(vm, src_reg1);
            Value start = get_register(vm, src_reg2);
            Value end = get_register(vm, src_reg3);
            Value step = get_register(vm, src_reg4);

            if (!IS_NUM(start) || !IS_NUM(end))
                vm_error(vm, "Slice [start] and [end] must be numbers.");

            if (!IS_NIL(step) && !IS_NUM(step))
                vm_error(vm, "Slice [step] must be nil or a number.");
            else
            {
                if (IS_SEQUENCE(sequence))
                {
                    double end_num = as_number(end);
                    Value slice = get_slice(AS_OBJ(sequence), as_number(start), as_number(end),
                                            IS_NIL(step) ? 1.0 : as_number(step));
                    set_register(vm, dest_reg, slice);
                }
                else
                    vm_error(vm, "Slice operand must be a list or string.");
            }
            break;
        }

        case OP_GET_ITEM:
        {
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            dest_reg = code[pc++];

            Value container = get_register(vm, src_reg1);
            Value index = get_register(vm, src_reg2);

            if (!IS_OBJ(container))
                vm_error(vm, "Unsupported operand type for get item operator.\n");

            switch (OBJ_TYPE(container))
            {
            case OBJ_LIST:
            {
                list_t *list = as_list(container);
                if (list->size == 0)
                    set_register(vm, dest_reg, NEW_NIL());
                else
                {
                    int _index = as_number(index);
                    Value item = *(Value *)list_getAt(list, _index);
                    set_register(vm, dest_reg, item);
                }
                break;
            }
            case OBJ_MAP:
            {
                Value item = map_get(AS_MAP(container), index);
                set_register(vm, dest_reg, item);
                break;
            }

            case OBJ_STRING:
            {
                char *str = as_string(container);
                int _index = get_index(as_number(index), strlen(str));

                char *_char = malloc(2);
                _char[0] = str[_index];
                _char[1] = '\0';
                set_register(vm, dest_reg, NEW_OBJ(add_obj(vm, new_pistring(_char))));
                free(str);
                break;
            }

            default:
                vm_error(vm, "Unsupported operand type for get item operator.\n");
            }
            break;
        }

        case OP_SET_ITEM:
        {
            src_reg1 = code[pc++];
            src_reg2 = code[pc++];
            src_reg3 = code[pc++];

            Value container = get_register(vm, src_reg1);
            Value index = get_register(vm, src_reg2);
            Value value = get_register(vm, src_reg3);

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
            src_reg1 = code[pc++];
            Value retval = get_register(vm, src_reg1);

            for (int i = vm->regfile->used_regs - 1; i >= vm->bp; i--)
                remove_upvalue(vm, i);

            Frame *frame = pop_frame(vm);

            while (vm->iter_sp > frame->iters_top)
                vm->iter_sp--;

            if (vm->iter_sp != -1)
                iter = vm->iters[vm->iter_sp];

            vm->pc = frame->pc;
            vm->bp = frame->bp;
            vm->code = frame->code;

            free_frame(frame);

            set_register(vm, 0, retval); // Result goes to register 0
            return;
        }

        case OP_HALT:
        {
            vm->running = false;
            return;
        }

        case OP_NO:
            break;

        case OP_PUSH_NIL:
        {
            dest_reg = code[pc++];
            set_register(vm, dest_reg, NEW_NIL());
            break;
        }

        case OP_DEBUG:
            printf("[DEBUG] Current PC: %d\n", pc);
            break;

        default:
            vm_errorf(vm, "Unknown opcode: [%d]\n", op);
            vm->pc = pc;
        }

#ifdef __EMSCRIPTEN__
        if (vm->counter >= vm->next_gc)
        {
            run_gc(vm);
            vm->counter = 0;
        }
#else
        if (vm->counter >= vm->next_gc)
        {
            int before = count_objs(vm);
            run_gc(vm);
            int after = count_objs(vm);
            int collected = before - after;

            vm->counter = 0;

            if (collected <= 0)
                vm->next_gc += vm->next_gc / 2;
            else
                vm->next_gc = after + (after / 2);
            vm->obj_count = after;

            if (vm->next_gc < GC_MIN_THRESHOLD)
                vm->next_gc = GC_MIN_THRESHOLD;
            else if (vm->next_gc > GC_MAX_THRESHOLD)
                vm->next_gc = GC_MAX_THRESHOLD;
        }
#endif
        vm->pc = pc;
    }
}

/**
 * Frees the memory allocated for a virtual machine instance.
 */
void free_vm(vm_t *vm)
{
    audio_stopAll();

    if (vm->cart)
    {
        cart_free(vm->cart);
    }
    
    // Free register file
    if (vm->regfile)
        free(vm->regfile);
    
    ht_free(vm->globals);
    pthread_mutex_destroy(&vm->lock);
    free(vm);
}