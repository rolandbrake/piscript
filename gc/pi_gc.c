
/* Important Note: trying to implement garbage collection using Tri-color marking
 tracing algorithm the code development still in progress */

#include "gc.h"
#include "list.h"
#include "pi_vm.h"
#include "pi_func.h"
// Forward declarations
static void mark_value(vm_t *vm, Value val);
static void mark_object(vm_t *vm, Object *obj);
static void process_objects(vm_t *vm); // process gray stack
static void make_black(vm_t *vm, Object *obj);
static void mark_roots(vm_t *vm);

/**
 * @brief Performs garbage collection on the virtual machine.
 *
 * This function initiates the garbage collection process by marking all roots,
 * processing the gray stack to mark reachable objects, and sweeping unreachable
 * objects. It also provides debug information about the number of objects before
 * and after collection if debugging is enabled.
 *
 * @param vm The virtual machine instance.
 */
void run_gc(vm_t *vm)
{
#ifdef DEBUG_GC
    printf("[GC] Starting collection (before: %d objects)\n", count_objs(vm));
#endif

    // Initialize gray stack if not already initialized
    if (!vm->gc_stack)
        vm->gc_stack = list_create(sizeof(Object *));

    // Mark phase: Mark all roots as reachable
    mark_roots(vm);

    // Process the gray stack to mark all reachable objects
    process_objects(vm);

    // Sweep phase: Free all unreachable objects
    sweep(vm);

    // Clear the gray stack for the next collection cycle
    list_clear(vm->gc_stack);

#ifdef DEBUG_GC
    printf("[GC] Collection complete (after: %d objects)\n", count_objs(vm));
#endif
}

/**
 * @brief Marks all constants in the virtual machine as reachable.
 *
 * This function iterates over the list of constants in the virtual machine
 * and marks each value as reachable. This is necessary to prevent the garbage
 * collector from freeing constants that are still in use.
 *
 * @param vm The virtual machine instance.
 */
void mark_constants(vm_t *vm)
{

    // Iterate over the list of constants
    int const_count = list_size(vm->constants);
    for (int i = 0; i < const_count; i++)
    {
        Value *val = (Value *)list_getAt(vm->constants, i);

        // Mark the value as reachable if it is not NULL
        if (val)
            mark_value(vm, *val);
    }
}

/**
 * Marks all roots in the virtual machine as reachable.
 *
 * This function traverses all possible roots in the virtual machine, such as
 * the stack, frames, global variables, open upvalues, iterators, and constants
 * to ensure that they are marked as reachable. This is the first step in the
 * garbage collection process to prevent accidental deallocation of live objects.
 *
 * @param vm The virtual machine instance.
 */
static void mark_roots(vm_t *vm)
{

    // Mark stack values
    for (int i = 0; i < vm->sp; i++)
        mark_value(vm, vm->stack[i]);

    // Mark all functions in the call stack
    for (int i = 0; i < vm->frame_sp; i++)
    {
        Frame *frame = vm->frames[i];
        if (frame && frame->function)
            mark_object(vm, (Object *)frame->function);
    }
    // Current function
    if (vm->function)
        mark_object(vm, vm->function);

    // Mark all global variables
    ht_iter it = ht_iterator(vm->globals);
    while (ht_next(&it))
    {
        Value *val = (Value *)it.value;
        if (val)
            mark_value(vm, *val);
    }

    // Mark open upvalues
    UpValue *up = vm->openUpvalues;
    while (up)
    {
        mark_value(vm, up->value);
        up = up->next;
    }

    // Mark iterators
    for (int i = 0; i <= vm->iter_sp; i++)
        if (vm->iters[i])
            mark_object(vm, vm->iters[i]);

    // Mark constants
    mark_constants(vm);
}

/**
 * Marks a Value as reachable.
 *
 * If the Value is an object, it marks the associated object as reachable.
 *
 * @param vm The virtual machine instance.
 * @param val The Value to be marked.
 */
static void mark_value(vm_t *vm, Value val)
{
    // Check if the value is an object
    if (IS_OBJ(val))
        // Mark the object as reachable
        mark_object(vm, AS_OBJ(val));
}

/**
 * Marks an object as reachable by setting its mark color to gray and
 * adding it to the gray stack.
 *
 * This function is called on objects that are in the white color
 * (i.e., not yet marked as reachable). If the object is already marked
 * (i.e., in the gray or black color), the function does nothing.
 *
 * @param vm The virtual machine.
 * @param obj The object to be marked.
 */
static void mark_object(vm_t *vm, Object *obj)
{
    if (!obj || obj->gc_color != GC_WHITE)
        return; // Object is already marked

    obj->gc_color = GC_GRAY;      // Mark as gray
    list_add(vm->gc_stack, &obj); // Add to gray stack

    for (int i = 0; i < list_size(vm->gc_stack); i++)
    {
        Object *_obj = *(Object **)list_getAt(vm->gc_stack, i);
        printf("%s\n", type_name(NEW_OBJ(_obj)));
    }
}

/**
 * Process the objects in the gray stack.
 *
 * This function will iterate over the gray stack and make each object black by
 * marking all of its references as reachable. This is the second phase of the
 * garbage collection process.
 *
 * @param vm The virtual machine instance containing the objects to be processed.
 */
static void process_objects(vm_t *vm)
{
    while (list_size(vm->gc_stack) > 0)
    {
        Object *obj = NULL;
        // Pop an object from the gray stack
        obj = *(Object **)list_pop(vm->gc_stack);
        obj->gc_color = GC_BLACK; // Mark as black
        // Make the object black by marking all of its references as reachable
        make_black(vm, obj);
    }
}

/**
 * Makes an object black by marking all of its references as reachable.
 * This function is called on objects that are in the gray stack and
 * are about to be marked black.
 *
 * @param vm The virtual machine instance containing the object to be processed.
 * @param obj The object to be processed.
 */
static void make_black(vm_t *vm, Object *obj)
{

    switch (obj->type)
    {
    case OBJ_LIST:
    {
        PiList *list = (PiList *)obj;
        int size = list_size(list->items);
        for (int i = 0; i < size; i++)
        {
            Value *val = (Value *)list_getAt(list->items, i);
            if (val)
                mark_value(vm, *val); // Mark all elements in the list as reachable
        }
        break;
    }

    case OBJ_MAP:
    {
        PiMap *map = (PiMap *)obj;
        ht_iter it = ht_iterator(map->table);
        while (ht_next(&it))
        {
            Value *val = (Value *)it.value;
            if (val)
                mark_value(vm, *val); // Mark all values in the map as reachable
        }
        break;
    }

    case OBJ_FUN:
    {
        Function *fn = (Function *)obj;
        if (fn->body)
            mark_object(vm, (Object *)fn->body); // Mark the body of the function as reachable

        if (fn->params)
        {
            int param_count = list_size(fn->params);
            for (int i = 0; i < param_count; i++)
            {
                Value *val = (Value *)list_getAt(fn->params, i);
                if (val)
                    mark_value(vm, *val); // Mark all parameters of the function as reachable
            }
        }

        if (fn->upvalues)
        {
            for (int i = 0; i < fn->upvalue_count; i++)
                if (fn->upvalues[i])
                    mark_object(vm, (Object *)fn->upvalues[i]); // Mark all upvalues of the function as reachable
        }

        if (fn->instance)
            mark_object(vm, fn->instance); // Mark the instance of the function as reachable
        break;
    }

    case OBJ_CODE:
    {
        ObjCode *code = (ObjCode *)obj;
        int size = list_size(code->data);
        for (int i = 0; i < size; i++)
        {
            Value *val = (Value *)list_getAt(code->data, i);
            if (val)
                mark_value(vm, *val); // Mark all bytecode instructions as reachable
        }
        break;
    }

    case OBJ_STRING:
        break;

    default:
        break;
    }
}

/**
 * Iterates over the linked list of objects in the virtual machine, freeing
 * any objects that have not been marked as reachable during the mark phase.
 *
 * @param vm The virtual machine instance containing the objects to be swept.
 */
void sweep(vm_t *vm)
{

    /**
     * Previous object in the linked list, used for updating the linked
     * list pointers when an object is freed.
     */
    Object *prev = NULL;

    /**
     * Current object in the linked list being processed.
     */
    Object *obj = vm->objects;

    while (obj)
    {
        /**
         * Next object in the linked list, used for updating the linked
         * list pointers when an object is freed.
         */
        Object *next = obj->next;

        if (obj->gc_color == GC_WHITE)
        {

            obj->in_gcList = false;
            /**
             * If the current object is unmarked (white), it is unreachable
             * and should be freed.
             */
            if (prev)
                prev->next = next;
            else
                vm->objects = next;

            free_object(obj);
        }
        else
        {
            obj->in_gcList = true;
            /**
             * If the current object is marked (gray or black), it is reachable
             * and should be left in the linked list. Reset the mark color
             * to white so it will be processed in the next garbage collection
             * cycle.
             */
            obj->gc_color = GC_WHITE;
            prev = obj;
        }

        obj = next;
    }
}

/**
 * Frees the allocated memory for an object based on its type.
 *
 * This function will deallocate the memory used by the object and any
 * associated data structures it contains, such as strings or lists.
 *
 * @param obj The object to be freed.
 */
void free_object(Object *obj)
{

    // Prevent stale GC tracking
    obj->in_gcList = false;

    switch (obj->type)
    {
    case OBJ_STRING:
    {
        // Free the memory allocated for the string characters
        PiString *string = (PiString *)obj;
        free(string->chars);
        break;
    }
    case OBJ_LIST:
    {
        // Free the memory allocated for the list items
        PiList *list = (PiList *)obj;

        list_free(list->items);

        break;
    }
    case OBJ_MAP:
    {
        // Free the memory allocated for the map items
        PiMap *map = (PiMap *)obj;

        ht_free(map->table);
        break;
    }

    case OBJ_CODE:
    {
        // Free the memory allocated for the code list
        ObjCode *code = (ObjCode *)obj;
        list_free(code->data);
        break;
    }

    case OBJ_FUN:
    {
        // Free the memory allocated for the function's code
        Function *function = (Function *)obj;
        free_func(function);
        break;
    }

    case OBJ_MODEL3D:
    {
        // Free the memory allocated for the model's triangles
        ObjModel3d *model = (ObjModel3d *)obj;
        free(model->triangles);
        break;
    }

    case OBJ_IMAGE:
    {
        // Free the memory allocated for the image's pixels and alpha channel
        ObjImage *image = (ObjImage *)obj;
        free(image->pixels);
        free(image->alpha);
        break;
    }
    default:
        // Handle other object types if needed
        break;
    }
    // Free the memory allocated for the object itself
    free(obj);
}