/******************************************************************************
** bob: Allocator for Bob objects, with garbage collection.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef OBJECTALLOCATOR_H
#define OBJECTALLOCATOR_H

#include "object.h"


BobObject* BobObject_alloc();


#endif /* OBJECTALLOCATOR_H */


