#ifndef BULK_ALLOCATOR_H
#define BULK_ALLOCATOR_H

#include <stddef.h>

// A simple custom memory allocator that mallocs a big chunk of memory
// and then hand it as requested
// With this allocator you cannot free memory until the end when the entire
// chunk is freed

typedef struct memory_chunk {
    void* start;
    void* current;
} memory_chunk;

void init_memory_chunk(memory_chunk* mc, size_t capacity);

void* allocate(memory_chunk* mc, size_t size);

void free_memory_chunk(memory_chunk* mc);

#endif // BULK_ALLOCATOR_H