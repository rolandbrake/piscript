#include <stdint.h>
#include "pi_func.h"
#include "pi_object.h"
// #include "pi_vm.h"

Object *new_func(char *name, list_t *body, list_t *params, UpValue **upvalues, Object *instance)
{
    // // Allocate memory for the function valueb
    // Value val = {0};

    // val.type = VAL_OBJ;
    Object *object = (Object *)malloc(sizeof(Function));

    object->type = OBJ_FUN;

    // Cast the allocated object to Function
    Function *fn = (Function *)object;

    // Assign function properties
    fn->name = name;
    fn->params = params ? params : list_create(sizeof(Value));
    fn->body = body ? body : list_create(sizeof(uint8_t));

    fn->is_native = false;
    fn->is_method = false;

    fn->native = NULL; // Explicitly set to NULL

    fn->upvalues = upvalues;

    fn->instance = instance;

    object->is_marked = false;

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
    // Value val = new_func(name, NULL, NULL); // Create a function object

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

    // fn->upvalue_count = 0;

    fn->is_native = true;
    fn->native = func;

    fn->instance = NULL;

    return val;
}

// Call a Function (default user-defined implementation)
Value call_func(vm_t *vm, Function *function, size_t argc, Value *argv)
{

    if (function->is_native)
        return function->native(vm, argc, argv);

    // Push the current frame onto the call stack
    Frame *frame = create_frame(vm->pc, vm->sp, vm->bp,
                                vm->code, vm->iter_sp, vm->ip);

    push_frame(vm, frame);

    // Update the VM state with the function's bytecode
    vm->code = function->body;

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

Value call_funcv(vm_t *vm, Function *function, size_t argc, ...)
{
    va_list args;
    va_start(args, argc);

    Value *argv = malloc(sizeof(Value) * argc);
    for (size_t i = 0; i < argc; i++)
        argv[i] = va_arg(args, Value);

    va_end(args);

    Value result = call_func(vm, function, argc, argv);
    free(argv); // Clean up

    return result;
}
void free_func(Function *fn)
{
    free(fn->name);
    list_free(fn->params);
    list_free(fn->body);
    free(fn);
}