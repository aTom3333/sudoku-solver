#include "bulk_allocator.h"
#include <stdlib.h>


void init_memory_chunk(memory_chunk* mc, size_t capacity)
{
    mc->start = malloc(capacity);
    mc->current = mc->start;
}

void* allocate(memory_chunk* mc, size_t size)
{
    // Simplest allocation possible, every memory allocation will be next to the other ones,
    // increasing the risk of corruption in case their is a buffer overrun but we expect our program
    // to be correct :)
    // We also don't check alignment, we would need to do that in a real allocator
    // but for this application, every allocation will has a size multiple of 8 so we
    // will always be 8-bytes aligned (hopefully)
    // Finally we don't check if we have exhausted the memory chunk, we expect the
    // program to be correct and reserve the correct amount of memory upfront
    void* p = mc->current;
    mc->current += size;
    return p;
}

void free_memory_chunk(memory_chunk* mc)
{
    // Free the whole chunk, every allocation made with this memory chunk is now free'd
    free(mc->start);
}
