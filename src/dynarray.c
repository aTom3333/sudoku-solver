//
// Created by thomas on 11/12/19.
//

#include "dynarray.h"
#include <stdlib.h>


// Simple dynamic array, not overly optimized because not used in hot path
struct dynarray {
    size_t capacity;
    size_t size;
    void** data;
    freeFn freeFunc;
};

dynarray* createDynArray(size_t capacity, freeFn freeFunction)
{
    dynarray* arr = malloc(sizeof(dynarray));
    arr->data = malloc(capacity * sizeof(void*));
    arr->capacity = capacity;
    arr->size = 0;
    arr->freeFunc = freeFunction;
}

void freeDynArray(dynarray* arr)
{
    size_t i;
    for(i = 0; i < getSizeDynArray(arr); i++) {
        arr->freeFunc(getAtDynArray(arr, i));
    }
    free(arr->data);
    free(arr);
}

inline size_t getSizeDynArray(dynarray* arr)
{
    return arr->size;
}

inline void* getAtDynArray(dynarray* arr, size_t index)
{
    return arr->data[index];
}

void append(dynarray* arr, void* elem)
{
    if(arr->size == arr->capacity) {
        arr->data = realloc(arr->data, arr->capacity * 1.5);
    }
    arr->data[arr->size++] = elem;
}
