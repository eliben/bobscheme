/******************************************************************************
** bob: Allocator for Bob objects, with garbage collection.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "objectallocator.h"
#include "mem.h"


BobObject* BobObject_alloc()
{
    return mem_alloc(sizeof(BobObject));
}
