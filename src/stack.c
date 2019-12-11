//
// Created by thomas on 05/12/19.
//

#include "stack.h"
#include <stdlib.h>


stack createStack(size_t capacity)
{
    stack result = {
        .data = malloc(capacity * sizeof(int32_t)),
        .size = 0
    };
    return result;
}

void freeStack(stack* st)
{
    free(st->data);
}

void push(stack* st, Node* node, HeaderNode* list)
{
    st->data[st->size++] = (void*)node - (void*)list;
}

Node* pop(stack* st, HeaderNode* list)
{
    return st->data[--st->size] + (void*)list;
}
