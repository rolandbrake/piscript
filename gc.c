#include "gc.h"
#include "list.h"
#include "pi_func.h"

void *reallocate(void *ptr, size_t o_size, size_t n_size)
{
    if (n_size == 0)
    {
        free(ptr);
        return NULL;
    }
    void *result = realloc(ptr, n_size);
    if (!result)
        exit(1);
    return result;
}

static void mark_globals(vm_t *vm)
{
    ht_iter it = ht_iterator(vm->globals);
    while (ht_next(&it))
    {
        Value *val = it.value;
        if (val != NULL)
            mark_value(*val);
    }
}

static void mark_iters(vm_t *vm)
{
    for (int i = 0; i <= vm->iter_sp; i++)
        if (vm->iters[i] != NULL)
            mark_object(vm->iters[i]);
}

void mark_constants(vm_t *vm)
{
    int count = list_size(vm->constants);
    for (int i = 0; i < count; i++)
        mark_value(*(Value *)list_getAt(vm->constants, i));
}
void mark_value(Value val)
{
    if (IS_OBJ(val))
        mark_object(AS_OBJ(val));
}

void mark_object(Object *obj)
{
    if (obj == NULL || obj->is_marked)
        return;
    obj->is_marked = true;
    // Recursively mark any referenced objects based on the object type.
    switch (obj->type)
    {
    case OBJ_LIST:
    {
        PiList *list = (PiList *)obj;
        int size = LIST_SIZE(list->items);
        for (int i = 0; i < size; i++)
        {
            Value *item = (Value *)list_getAt(list->items, i);
            if (item)
                mark_value(*item); // Ensure items inside lists are also marked
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
            if (val != NULL)
                mark_value(*val);
        }
        break;
    }
    case OBJ_FUN:
    {
        Function *function = (Function *)obj;
        // Mark the function's parameters
        for (int i = 0; i < list_size(function->params); i++)
        {
            Value *param = (Value *)list_getAt(function->params, i);
            if (param)
                mark_value(*param);
        }
        break;
    }
    default:
        break;
    }
}

void sweep(vm_t *vm)
{
    Object *prev = NULL;
    Object *obj = vm->objects;
    while (obj != NULL)
    {
        Value val = NEW_OBJ(obj);
        // printf("[GC] Sweeping object at %p\n", (void *)obj);
        // print_value(val, true);
        // printf("[GC] Freeing object of type %s\n", type_name(val));
        if (!obj->is_marked)
        {
            Object *next = obj->next;
            free_object(obj);

            obj = NULL; // Prevent accessing freed memory

            if (prev != NULL)
                prev->next = next; // Update previous object's next pointer
            else
                vm->objects = next; // Update head of list if necessary
            obj = next;
        }
        else
        {
            obj->is_marked = false;
            prev = obj;
            obj = obj->next;
        }
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

    case OBJ_FUN:
    {
        // Free the memory allocated for the function's code
        Function *function = (Function *)obj;
        free_func(function);
        break;
    }
    default:
        // Handle other object types if needed
        break;
    }
    // Free the memory allocated for the object itself
    free(obj);
}

void free_value(vm_t *vm, Value *val)
{
    if (val == NULL)
        return;

    if (val->type == VAL_OBJ)
        free_object(AS_OBJ(*val)); // Properly free objects (lists, strings, etc.)

    free(val); // Free the allocated Value struct
}

// Mark all roots: typically the VM’s stack, global variables, etc.
void mark_roots(vm_t *vm)
{

    // Mark all values on the VM stack.
    for (int i = 0; i < vm->sp; i++)
        mark_value(vm->stack[i]);

    // If your VM has global variables or other roots (e.g. open upvalues, etc.),
    // mark them here as well.
}

static void print_object_chain(vm_t *vm)
{
    printf("\n[DEBUG] Object Chain:\n");
    Object *obj = vm->objects;
    while (obj)
    {
        printf("  - Object at %p (Type: %s, Next: %p)\n", (void *)obj, type_name(NEW_OBJ(obj)), (void *)obj->next);
        obj = obj->next;
    }
}

/**
 * Run a full garbage collection cycle.
 *
 * This will mark all reachable objects (by traversing the roots), and then
 * sweep the heap to free any unreachable objects.
 *
 * @param vm The virtual machine instance.
 */
void run_gc(vm_t *vm)
{

    mark_globals(vm);
    mark_iters(vm);
    // Mark all roots: typically the VM's stack, global variables, etc.
    mark_roots(vm);

    // // Sweep the heap to free any unreachable objects.
    sweep(vm);
}
