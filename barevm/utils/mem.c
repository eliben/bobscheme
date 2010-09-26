#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "die.h"


static void die_oom(size_t size)
{
    die("Out of memory, malloc failed for %u bytes\n", size);
}


void* mem_alloc(size_t size)
{
    void* ptr = malloc(size);
    
    if (!ptr)
        die_oom(size);
    
    return ptr;
}


void* mem_calloc(size_t n, size_t size)
{
    void* ptr = calloc(n, size);
    
    if (!ptr)
        die_oom(size);
    
    return ptr;
}


void* mem_realloc(void* ptr, size_t size)
{
    void* newptr = realloc(ptr, size);
    
    if (!newptr)
        die_oom(size);
    
    return newptr;
}


void mem_free(void* ptr)
{
    if (ptr)
        free(ptr);
}


void* mem_try_alloc(size_t size)
{
    return malloc(size);
}


void* mem_try_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}


void* mem_try_calloc(size_t n, size_t size)
{
    return calloc(n, size);
}
