#ifndef STRING_H
#define STRING_H

#include "list.h"

#define GET_STRING(STR) ((String *)STR)->data

typedef struct
{
    char *data; // Pointer to dynamically allocated string
    int length; // Store string length for convenience
} String;


String *new_string(char *data);
char *string_get(list_t *list, int index);
void free_strings(list_t *list);
void free_string(String *str);

#endif // STRING_H