//
// Created by thomas on 05/12/19.
//

#include "stack.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct stack {
    int32_t* data;
};

stack* createStack(size_t capacity)
{
    stack* st = malloc(sizeof(struct stack));
    st->data = malloc((capacity+1) * sizeof(int32_t));
    st->data[0] = 0;
    return st;
}

void freeStack(stack* st)
{
    free(st->data);
    free(st);
}

void push(stack* st, Node* node, HeaderNode* list)
{
    st->data[++(st->data[0])] = (void*)node - (void*)list;
}

Node* pop(stack* st, HeaderNode* list)
{
    return st->data[st->data[0]--] + (void*)list;
}

inline size_t getSize(stack* st)
{
    return st->data[0];
}

inline Node* getAt(stack* st, size_t index, HeaderNode* list)
{
    return st->data[index+1] + (void*)list;
}

void* getRawData(stack* st)
{
    return st->data;
}

stack* fromRawData(void* data)
{
    stack* st = malloc(sizeof(struct stack));
    st->data = data;
    return st;
}

stack* copyStack(stack* src, size_t capacity)
{
    stack* dst = malloc(sizeof(struct stack));
    dst->data = malloc((capacity+1) * sizeof(int32_t));
    memcpy(dst->data, src->data, (src->data[0]+1)* sizeof(int32_t));
    return dst;
}

void printStack(stack* st)
{
    printf("size: %d\n", st->data[0]);
    int i;
    for(i = 0; i < st->data[0]; i++) {
        printf("\t%d", st->data[i+1]);
    }
    puts("");
}
