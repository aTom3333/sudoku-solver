#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>
#include "node.h"

struct stack;
typedef struct stack stack;

stack* createStack(size_t capacity);
void freeStack(stack* st);

void push(stack* st, Node* node, HeaderNode* list);
Node* pop(stack* st, HeaderNode* list);

inline size_t getSize(stack* st);
inline Node* getAt(stack* st, size_t index, HeaderNode* list);

void* getRawData(stack* st);
stack* fromRawData(void* data);

stack* copyStack(stack* src, size_t capacity);

#endif // STACK_H