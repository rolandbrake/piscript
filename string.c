#include <stdlib.h>
#include <string.h>

#include "string.h"

String *new_string(char *data)
{
    String *result = malloc(sizeof(String));
    result->data = strdup(data);
    result->length = strlen(data);
    return result;
}

char *string_get(list_t *list, int index)
{
    String *str = (String *)list_getAt(list, index);
    return str->data;
}

void free_strings(list_t *list)
{

    for (int i = LIST_SIZE(list) - 1; i >= 0; i--)
    {
        String *str = (String *)list_getAt(list, i);
        printf("Freeing string: %s\n", str->data);
        if (str)
        {
            free(str->data); // Safe even if str->data is NULL
            free(str);
        }
    }
}

void free_string(String *str)
{
    free(str->data);
    free(str);
}