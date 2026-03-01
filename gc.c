#include "gc.h"
#include "list.h"
#include "pi_func.h"

/**
 * @brief Marks all values in a list as reachable.
 *
 * This function iterates over the list and marks each value
 * as reachable. This is necessary for the garbage collector
 * to know not to free the memory associated with any of the
 * values in the list.
 *
 * @param list The list whose values are to be marked.
 */
void mark_list(list_t *list)
{
    if (!list)
        return; // If the list is NULL, exit the function

    int size = list_size(list);
    for (int i = 0; i < size; i++)
    {
        Value *val = (Value *)list_getAt(list, i); // Get the value at index i
        if (val)
            mark_value(*val); // Mark the value if it's not NULL
    }
}

/**
 * @brief Reallocates a memory block to a new size.
 *
 * This function changes the size of the memory block pointed to by `ptr`.
 * If the new size (`n_size`) is zero, the memory is freed.
 * If the reallocation fails, the program exits with an error code.
 *
 * @param ptr Pointer to the currently allocated memory block.
 * @param o_size The original size of the memory block (unused in this function).
 * @param n_size The new size for the memory block.
 * @return A pointer to the newly allocated memory block, or NULL if `n_size` is zero.
 */
void *reallocate(void *ptr, size_t o_size, size_t n_size)
{
    if (n_size == 0)
    {
        free(ptr); // Free the memory if the new size is zero
        return NULL;
    }
    void *result = realloc(ptr, n_size); // Attempt to reallocate memory
    if (!result)
        exit(1); // Exit if reallocation fails
    return result;
}

/**
 * @brief Marks all values in the global hash table as reachable.
 *
 * This function iterates over the global hash table and marks each value
 * as reachable. This is necessary so that the garbage collector knows not
 * to free the memory associated with any of the values in the global hash
 * table.
 *
 * @param vm The virtual machine.
 */
void mark_globals(vm_t *vm)
{
    ht_iter it = ht_iterator(vm->globals);
    while (ht_next(&it))
    {
        Value *val = it.value;
        if (val != NULL)
            mark_value(*val);
    }
}

/**
 * @brief Marks all iterators in the iterator stack as reachable.
 *
 * This function iterates over the iterator stack and marks each iterator
 * as reachable. This is necessary so that the garbage collector knows not
 * to free the memory associated with any of the iterators in the iterator
 * stack.
 *
 * @param vm The virtual machine.
 */
void mark_iters(vm_t *vm)
{
    for (int i = 0; i <= vm->iter_sp; i++)
    {
        if (vm->iters[i] != NULL)
            mark_object(vm->iters[i]);
    }
}

void mark_constants(vm_t *vm)
{
    int count = list_size(vm->constants);
    for (int i = 0; i < count; i++)
        mark_value(*(Value *)list_getAt(vm->constants, i));
}

/**
 * @brief Marks a value as reachable.
 *
 * If the value is an object, marks the object as reachable.
 *
 * @param val The value to mark.
 */
void mark_value(Value val)
{
    if (IS_OBJ(val))
        mark_object(AS_OBJ(val));
}

/**
 * @brief Marks an object as reachable.
 *
 * If the object is not marked, marks it as reachable and recursively marks any
 * referenced objects based on the object type.
 *
 * @param obj The object to mark.
 */
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
        // Mark the elements of a list
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

        if (map->proto)
            mark_object((Object *)map->proto);

        table_t *table = map->table;
        if (!table)
            break;

        for (int i = 0; i < table->capacity; i++)
        {
            ht_item *item = &table->items[i];
            if (!item->key || !item->value)
                continue;

            Value *val = (Value *)item->value;
            if (val && IS_OBJ(*val))
                mark_object(AS_OBJ(*val));
        }
        break;
    }

    case OBJ_CODE:
    {
        ObjCode *code = (ObjCode *)obj;
        mark_list(code->data); // if necessary
        break;
    }

    case OBJ_FUN:
    {
        Function *fn = (Function *)obj;

        mark_list(fn->params);

        if (fn->body)
            mark_object((Object *)fn->body);

        if (fn->upvalues)
            for (int i = 0; i < fn->upvalue_count; i++)
                mark_object((Object *)fn->upvalues[i]);

        if (fn->instance)
            mark_object(fn->instance);

        break;
    }

    default:
        break;
    }
}

/**
 * @brief Sweeps through the VM's object list, freeing unmarked objects.
 *
 * This function iterates over the linked list of objects in the virtual machine.
 * It checks each object to see if it is marked. Unmarked objects are considered
 * unreachable and are freed. The function also updates the linked list to remove
 * the freed objects. Marked objects are prepared for the next garbage collection
 * cycle by resetting their mark status.
 *
 * @param vm The virtual machine instance containing the objects to be swept.
 */

/**
 * @brief Sweeps through the VM's object list, freeing unmarked objects.
 *
 * This function iterates over the linked list of objects in the virtual machine.
 * It checks each object to see if it is marked. Unmarked objects are considered
 * unreachable and are freed. The function also updates the linked list to remove
 * the freed objects. Marked objects are prepared for the next garbage collection
 * cycle by resetting their mark status.
 *
 * @param vm The virtual machine instance containing the objects to be swept.
 */
void sweep(vm_t *vm)
{

    Object *obj = vm->objects;
    Object *prev = NULL;

    while (obj != NULL)
    {
        Object *next = obj->next;

        if (!obj->is_marked)
        {
            // If the object is unmarked, it is unreachable and should be freed
            obj->in_gcList = false; // Reset the GC tracking flag

            free_object(obj); // Free the memory of the unmarked object

            // Remove the object from the linked list
            if (prev != NULL)
                prev->next = next;
            else
                vm->objects = next;
        }
        else
        {
            obj->is_marked = false; // Reset the mark for the next GC cycle
            obj->in_gcList = true;  // Ensure it remains in the GC list
            prev = obj;             // Move prev to current object
        }

        obj = next; // Move to the next object in the list
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

    obj->in_gcList = false; // Prevent stale GC tracking

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
        // Free the memory allocated for the map's key-value pairs
        PiMap *map = (PiMap *)obj;
        // Values stored in the table are plain Value cells. Any nested
        // objects are owned by the VM object list and must not be freed here.
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
    case OBJ_SPRITE:
    {
        ObjSprite *sprite = (ObjSprite *)obj;
        free(sprite->data);
        break;
    }

    default:
        // Handle other object types if needed
        break;
    }
    // Free the memory allocated for the object itself
    free(obj);
}

/**
 * @brief Frees the allocated memory for a Value.
 *
 * This function checks the type of the Value and frees any associated
 * objects if necessary. It then frees the Value struct itself.
 *
 * @param val The Value to be freed.
 */
void free_value(Value *val)
{
    if (val == NULL)
        return;

    // If the value is an object, free the associated object.
    if (val->type == VAL_OBJ)
        free_object(AS_OBJ(*val));

    // Free the allocated Value struct.
    free(val);
}

/**
 * Marks all roots: typically the VM's stack, global variables, etc.
 *
 * This function traverses all reachable objects from the roots and marks them
 * as live. This is the first step of the garbage collection process.
 */
void mark_roots(vm_t *vm)
{
    // Stack values
    for (int i = 0; i < vm->sp; i++)
        mark_value(vm->stack[i]);

    // If your VM has global variables or other roots (e.g. open upvalues, etc.),
    // mark them here as well.

    for (int i = 0; i < vm->frame_sp; i++)
    {
        Frame *frame = vm->frames[i];
        if (frame != NULL && frame->function != NULL)
            mark_object((Object *)frame->function);
    }

    // Current function
    if (vm->function)
        mark_object(vm->function);

    // Open upvalues (linked list)
    UpValue *up = vm->openUpvalues;
    while (up != NULL)
    {
        mark_value(up->value);
        up = up->next;
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
    mark_constants(vm);

    // Mark all roots: typically the VM's stack, global variables, etc.
    mark_roots(vm);

    // Sweep the heap to free any unreachable objects.
    sweep(vm);
}

/**
 * Prints out the object chain, including the memory address of each object, its
 * type, and the address of the next object in the chain.
 *
 * @param vm The virtual machine instance.
 */
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

