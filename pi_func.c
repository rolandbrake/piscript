#include <stdint.h>
#include "pi_func.h"
#include "pi_object.h"

/**
 * Create a new function object.
 *
 * This function creates and initializes a new function object, which can be
 * either a user-defined function or a method bound to an instance.
 *
 * @param name The name of the function.
 * @param body The bytecode instructions of the function.
 * @param params The list of parameters for the function.
 * @param upvalues The list of upvalues used by the function.
 * @param instance The bound instance for methods, or NULL for standalone functions.
 * @return A pointer to the newly created function object.
 */
Object *new_func(char *name, ObjCode *body, list_t *params, UpValue **upvalues, Object *instance)
{

    // Allocate and verify memory
    Object *object = (Object *)malloc(sizeof(Function));

    // Initialize object header
    object->type = OBJ_FUN;
    object->is_marked = false;

    Function *fn = (Function *)object;

    // Handle function name (make copy if needed)
    fn->name = name ? strdup(name) : strdup("<FUN>");

    // Handle parameters
    fn->params = params ? params : list_create(sizeof(Value));

    // Set function body
    fn->body = body;

    // Set function flags
    fn->is_native = false;
    fn->is_method = false;
    fn->native = NULL;

    // Handle upvalues
    fn->upvalues = upvalues;
    fn->instance = instance;

    // Count upvalues
    int count = 0;
    if (upvalues)
        while (upvalues[count] != NULL)
            count++;

    fn->upvalue_count = count;

    return object;
}

/**
 * Create a new native function.
 *
 * A native function is a function that is not user-defined. It is a function
 * that is defined by the interpreter itself. Native functions are used to
 * implement the built-in functions of the language.
 *
 * @param name The name of the native function.
 * @return A new native function.
 */
Value *new_native(const char *name, native_func func)
{

    Value *val = malloc(sizeof(Value));
    val->type = VAL_OBJ;
    val->data.object = (Object *)malloc(sizeof(Function));

    val->data.object->type = OBJ_FUN;
    val->data.object->is_marked = true;

    // Cast the allocated object to Function
    Function *fn = (Function *)val->data.object;

    // Assign function properties
    fn->name = strdup(name); // Allocate and copy name string

    fn->params = NULL;
    fn->body = NULL;

    fn->is_native = true;
    fn->native = func;

    fn->instance = NULL;

    return val;
}

// Call a Function (default user-defined implementation)
/**
 * Calls a user-defined or native function. The function is either a native
 * function defined by the interpreter or a user-defined function.
 *
 * @param vm The current VM state.
 * @param function The function to call.
 * @param argc The number of arguments to pass to the function.
 * @param argv The arguments to pass to the function.
 * @return The return value of the function.
 */
Value call_func(vm_t *vm, Function *function, size_t argc, Value *argv)
{
    // If the function is a native function, call it directly
    if (function->is_native)
        return function->native(vm, argc, argv);

    // Push the current frame onto the call stack
    Frame *frame = create_frame(vm->pc, vm->sp, vm->bp,
                                vm->code, vm->iter_sp, vm->ip, function);

    push_frame(vm, frame);

    // Update the VM state with the function's bytecode
    vm->code = function->body->data;

    vm->pc = 0;
    vm->ip = 0;
    vm->bp = vm->sp;
    vm->sp = vm->bp + list_size(function->params);

    // Bind the function instance (if present)
    if (function->is_method)
    {
        Value instance = function->instance == NULL ? NEW_NIL() : NEW_OBJ(add_obj(vm, function->instance));
        vm->stack[vm->bp] = instance;
        // Prepare arguments with 'this' as the first argument
        Value *fargs = (Value *)malloc(sizeof(Value) * (argc + 1));
        fargs[0] = instance; // 'this' reference

        memcpy(fargs + 1, argv, sizeof(Value) * argc);

        argv = fargs;
        argc++;
    }
    list_t *_args = list_create(sizeof(Value));

    // Set function parameters and arguments
    for (size_t i = 0; i < argc; i++)
    {
        vm->stack[vm->bp + i] = argv[i];
        list_add(_args, &argv[i]);
    }

    for (size_t i = argc; i < function->params->size; i++)
    {
        Value _default = *(Value *)list_getAt(function->params, i);
        vm->stack[vm->bp + i] = _default;
    }

    vm->stack[vm->sp] = NEW_OBJ(add_obj(vm, new_list(_args)));

    // Start executing the function body
    vm->sp++;

    run(vm);

    // Pop the return value from the stack
    vm->sp--;
    return vm->stack[vm->sp];
}

/**
 * Call a function with variable arguments.
 *
 * This function is a wrapper around the regular call_func function that takes
 * a variable number of arguments.
 *
 * @param vm The current VM state.
 * @param function The function to call.
 * @param argc The number of arguments to pass to the function.
 * @param ... The arguments to pass to the function.
 * @return The return value of the function.
 */
Value call_funcv(vm_t *vm, Function *function, size_t argc, ...)
{
    va_list args;
    va_start(args, argc);

    Value *argv = malloc(sizeof(Value) * argc);
    for (size_t i = 0; i < argc; i++)
        argv[i] = va_arg(args, Value);

    va_end(args);

    // Call the function with the prepared arguments
    Value result = call_func(vm, function, argc, argv);

    // Clean up after ourselves
    free(argv);

    return result;
}
/**
 * Free the memory allocated for a function.
 *
 * @param fn The function to free.
 */
void free_func(Function *fn)
{
    free(fn->name);        // Free the function name
    list_free(fn->params); // Free the parameter list
}
