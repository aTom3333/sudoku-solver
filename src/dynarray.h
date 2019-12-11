#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h>

struct dynarray;
typedef struct dynarray dynarray;

typedef void(*freeFn)(void*);

dynarray* createDynArray(size_t capacity, freeFn freeFunction);
void freeDynArray(dynarray*);

void append(dynarray* arr, void* elem);

inline size_t getSizeDynArray(dynarray* arr);
inline void* getAtDynArray(dynarray* arr, size_t index);

#endif // DYNARRAY_H