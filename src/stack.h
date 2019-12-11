#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>
#include "node.h"

typedef struct stack {
    int32_t* data;
    size_t size;
} stack;

stack createStack(size_t capacity);
void freeStack(stack* st);

void push(stack* st, Node* node, HeaderNode* list);
Node* pop(stack* st, HeaderNode* list);

#endif // STACK_H