#ifndef MEM_H
#define MEM_H

#include <stddef.h>


/* Allocation functions with out-of-memory handling.
*/
void* mem_alloc(size_t size);
void* mem_realloc(void* ptr, size_t size);
void* mem_calloc(size_t n, size_t size);


/* Similar to the standard functions. In case of out-of-memory, 
** NULL is returned.
*/
void* mem_try_alloc(size_t size);
void* mem_try_realloc(void* ptr, size_t size);
void* mem_try_calloc(size_t n, size_t size);

void mem_free(void* ptr);


#endif /* MEM_H */
