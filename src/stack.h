#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include "node.h"

typedef struct stack {
    void** data;
    size_t size;
} stack;

stack createStack(size_t capacity);
void freeStack(stack* st);

void push(stack* st, void* value);
void* pop(stack* st);

#endif // STACK_H