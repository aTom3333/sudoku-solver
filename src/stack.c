//
// Created by thomas on 05/12/19.
//

#include "stack.h"
#include <stdlib.h>


stack createStack(size_t capacity)
{
    stack result = {
        .data = malloc(capacity * sizeof(void*)),
        .size = 0
    };
    return result;
}

void freeStack(stack* st)
{
    free(st->data);
}

void push(stack* st, void* value)
{
    st->data[st->size++] = value;
}

void* pop(stack* st)
{
    return st->data[--st->size];
}
