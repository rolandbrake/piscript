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
    for (int i = 0; i < LIST_SIZE(list); i++)
    {
        String *str = (String *)list_getAt(list, i);
        free(str->data);
        free(str);
    }
}

void free_string(String *str)
{
    free(str->data);
    free(str);
}