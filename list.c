#include "list.h"
#include "common.h"
#include "pi_value.h"
#include "pi_object.h"

/**
 * @brief Allocates memory for a new list with the given item size and capacity.
 *
 * The `capacity` parameter is the maximum number of items that can be stored in
 * the list without resizing.
 *
 * @param item_size The size of each item in the list.
 * @param capacity The initial capacity of the list.
 * @return A pointer to the newly created list, or NULL if memory allocation
 * fails.
 */
static list_t *_list_create(int item_size, int capacity)
{
    list_t *list = (list_t *)malloc(sizeof(list_t));
    if (!list)
    {
        perror("Failed to allocate memory for list");
        exit(EXIT_FAILURE);
    }

    list->data = malloc(item_size * capacity);
    if (!list->data)
    {
        free(list);
        perror("Failed to allocate memory for list data");
        exit(EXIT_FAILURE);
    }

    list->i_size = item_size;
    list->size = 0;
    list->capacity = capacity;

    return list;
}

/**
 * @brief Creates a new list with the given item size and a default capacity.
 *
 * The default capacity is currently 8.
 *
 * @param i_size The size of each item in the list.
 * @return A pointer to the newly created list.
 */
list_t *list_create(int i_size)
{
    return _list_create(i_size, INIT_CAP);
}

/**
 * @brief Adds an item to the end of the list.
 *
 * This function automatically expands the list's capacity if the list is
 * currently full. It then copies the item into the list's data array.
 *
 * @param list The list to add the item to.
 * @param item A pointer to the item to be added to the list.
 */
void list_add(list_t *list, const void *item)
{
    if (list->size == list->capacity)
    {
        // Use a hybrid strategy to determine the new capacity
        if (list->capacity < 1024) // Small lists: double the capacity
            list_expand(list, list->capacity * 2);
        else // Large lists: increase by 25% or at least 256
            list_expand(list, list->capacity + list->capacity / 4 + 256);
    }

    // Copy the item into the list's data array
    void *target = (byte *)list->data + list->size * list->i_size;
    memcpy(target, item, list->i_size);

    // Increment the size of the list
    list->size++;
}

/**
 * @brief Inserts an item into the list at the specified index.
 *
 * This function shifts existing elements to the right to make
 * room for the new item.
 *
 * @param list The list to insert the item into.
 * @param index The index at which to insert the item. If the index
 *              is negative or greater than the list size, it will
 *              be adjusted accordingly.
 * @param item A pointer to the item to be inserted into the list.
 */
void list_addAt(list_t *list, int index, const void *item)
{
    // Adjust index for negative values or values greater than the current size
    int _index = get_index(index, list->size);

    // Calculate the target position in memory
    void *target = (byte *)list->data + _index * list->i_size;

    // Shift elements to the right to make space for the new item
    memmove(target + list->i_size, target, (list->size - _index) * list->i_size);

    // Copy the new item into the target position
    memcpy(target, item, list->i_size);

    // Update the size of the list
    list->size++;
}

/**
 * @brief Retrieves the element at the specified index.
 *
 * @param list The list to retrieve an element from.
 * @param index The index of the element to retrieve.
 * @return The element at the specified index. If the index is out of bounds,
 *         a null pointer is returned.
 */
void *list_getAt(list_t *list, int index)
{
    // Adjust the index to be within bounds
    int _index = get_index(index, list->size);
    // Return a pointer to the element at the specified index
    return (byte *)list->data + _index * list->i_size;
}

/**
 * @brief Sets the element at the specified index to the given item.
 *
 * This function currently does not handle out-of-bounds access. If the index
 * is negative or greater than or equal to the list's size, the function will
 * exit without doing anything. This behavior is subject to change.
 *
 * @param list The list to modify.
 * @param index The index of the element to set.
 * @param item The new value for the element.
 */
void list_set(list_t *list, int index, void *item)
{
    int _index = get_index(index, list->size);

    void *target = (byte *)list->data + (_index * list->i_size);
    memcpy(target, item, list->i_size);
}

/**
 * @brief Returns the number of elements in the list.
 *
 * @param list The list to get the size of.
 * @return The number of elements in the list.
 */
int list_size(list_t *list)
{
    return list->size;
}

/**
 * @brief Creates a deep copy of the given list.
 *
 * This function allocates a new list with the same capacity and copies each
 * element from the original list to the new list. Each element is copied
 * using `copy_value` to ensure a deep copy.
 *
 * @param list The list to be copied.
 * @return A new list containing copies of the elements from the original list.
 */
list_t *list_copy(list_t *list)
{
    // Create a new list with the same capacity as the original
    list_t *copy = _list_create(sizeof(Value), list->capacity);

    // Iterate over each item in the original list
    for (size_t i = 0; i < list->size; i++)
    {
        Value *item = (Value *)list_getAt(list, i); // Get item at index i
        Value _item = copy_value(*item);            // Create a deep copy of the item
        list_add(copy, &_item);                     // Add the copied item to the new list
    }

    return copy; // Return the newly created list copy
}

/**
 * @brief Adds all elements from a list to the end of another list.
 *
 * This function will expand the destination list if necessary to accommodate
 * the new elements. The elements from the source list are copied to the end
 * of the destination list, and the size of the destination list is updated
 * accordingly.
 *
 * @param list The destination list to add elements to.
 * @param items The source list containing the elements to add.
 * @return The destination list with the added elements.
 */
list_t *list_addAll(list_t *list, list_t *items)
{

    // Calculate the required capacity after adding the new elements
    int size = list->size + items->size;

    if (size > list->capacity)
    {
        // Expand the list to accommodate the new elements
        int capacity = (size > list->capacity * 2) ? size : list->capacity * 2;
        list_expand(list, capacity);
    }

    // Copy the elements from 'items' to 'list'
    void *dest = (char *)list->data + list->size * list->i_size;
    memcpy(dest, items->data, items->size * items->i_size);

    // Update the size of the list
    list->size = size;

    return list;
}

/**
 * @brief Removes an element from the list at the specified index.
 *
 * This function removes an element from the list at the given index and returns
 * a pointer to the removed element. The list's size is decreased by one. The caller
 * is responsible for freeing the memory allocated for the returned element.
 *
 * @param list The list from which to remove the element.
 * @param index The index of the element to remove.
 * @return A pointer to the removed element.
 */

void *list_remove(list_t *list, int index)
{
    index = get_index(index, list->size);

    void *target = (byte *)list->data + index * list->i_size;

    // Allocate temporary buffer to hold the removed item
    void *removed_item = malloc(list->i_size);

    void *next = (byte *)list->data + (index + 1) * list->i_size;
    memmove(target, next, (list->size - index - 1) * list->i_size);

    list->size--;

    return removed_item; // Caller is responsible for freeing this
}

/**
 * @brief Removes and returns the last element of the list.
 *
 * This function removes and returns the last element of the list. The list's size is
 * decreased by one. The caller is responsible for freeing the memory allocated for
 * the returned element.
 *
 * @param list The list from which to remove the element.
 * @return A pointer to the removed element.
 */
void *list_pop(list_t *list)
{
    if (list->size == 0)
        error("[pop] PiList is empty.");

    list->size--;
    void *item = (byte *)list->data + list->size * list->i_size;
    return item;
}

/**
 * @brief Prepends an item to the beginning of the list.
 *
 * This function adds the given item to the beginning of the list. If the list is
 * already at capacity, it is first expanded to double its size (or increased by
 * 25% if the current capacity is greater than 1024). The existing items in the
 * list are shifted one position forward and the new item is inserted at the
 * beginning of the list.
 *
 * @param list The list to which to prepend the item.
 * @param item The item to prepend to the list.
 */
void list_addFirst(list_t *list, const void *item)
{
    if (list->size == list->capacity)
        // If the list is at capacity, expand it to double its size or increase by 25%
        list_expand(list, list->capacity < 1024 ? list->capacity * 2 : list->capacity + list->capacity / 4 + 256);

    // Move all existing items one slot forward
    void *dest = (byte *)list->data + list->i_size;
    void *src = list->data;
    memmove(dest, src, list->size * list->i_size);

    // Insert the new item at the beginning
    memcpy(list->data, item, list->i_size);
    list->size++;
}

/**
 * @brief Resizes the list to a new capacity.
 *
 * This function updates the capacity of the list to the specified
 * new capacity and reallocates memory for the list's data array.
 * If the reallocation fails, it prints an error message and exits
 * the program.
 *
 * @param list The list to be resized.
 * @param new_cap The new capacity for the list.
 */
void list_expand(list_t *list, int new_cap)
{
    void *_value = realloc(list->data, new_cap * list->i_size);
    if (_value == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    list->data = _value;
    list->capacity = new_cap;
}
/**
 * Applies a transformation function to each element in the list and returns a new list with the transformed elements.
 *
 * @param list The original list to be mapped over.
 * @param func A pointer to the function that transforms each element.
 * @return A new list containing the transformed elements, or NULL if the input is invalid.
 */
list_t *list_map(list_t *list, Value *(*func)(Value *))
{
    // Validate input arguments
    if (!list || !func)
        error("Invalid arguments to list_map.");

    // Create a new list to store the mapped values
    list_t *_list = list_create(list->i_size);
    for (int i = 0; i < list->size; i++)
    {
        // Get the current item, apply the transformation, and add to the new list
        Value *item = (Value *)list_getAt(list, i);
        Value *mapped = func(item);
        list_add(_list, mapped);
    }
    return _list;
}

/**
 * @brief Checks if the list is empty.
 *
 * This function takes a list as an argument and returns true if the list is empty, false otherwise.
 *
 * @param list The list to be checked.
 * @return true if the list is empty, false otherwise.
 */
bool list_isEmpty(list_t *list)
{
    return list->size == 0;
}

/**
 * @brief Clears all elements from the list without deallocating memory.
 *
 * This function resets the list's size to zero, effectively clearing all
 * elements. If the list contains pointers to dynamically allocated memory,
 * additional code should be added to free each element to prevent memory leaks.
 *
 * @param list The list to be cleared.
 */
void list_clear(list_t *list)
{
    if (!list || !list->data)
        return; // Exit if the list or its data is NULL

    // Note: If elements are pointers and need to be freed, uncomment and implement as needed:
    // for (int i = 0; i < list->size; i++) {
    //     free(((void **)list->data)[i]); // Free each element if dynamically allocated
    // }

    list->size = 0; // Reset the size of the list to zero

    // Optional: Zero out the memory for debugging/clean state
    memset(list->data, 0, list->capacity * list->i_size);
}

void list_print(list_t *list)
{
    for (int i = 0; i < list->size; i++)
    {
        Value item = *(Value *)list_getAt(list, i);
        printf("%s\n", as_string(item));
    }
}
/**
 * Frees the memory allocated for a list.
 *
 * @param list The list to free.
 */
void list_free(list_t *list)
{

    if (!list)
        return; // Avoid freeing NULL

    // Free the memory allocated for the data
    if (list->data)
    {
        free(list->data);  // Free the memory allocated for the list
        list->data = NULL; // Set the data pointer to NULL
    }

    list->size = 0;     // Set the size to 0
    list->capacity = 0; // Set the capacity to 0
    list->i_size = 0;   // Set the item size to 0

    free(list); // Free the memory allocated for the list
}
