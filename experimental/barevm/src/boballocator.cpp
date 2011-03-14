//*****************************************************************************
// bob: Bob object allocator & garbage collector
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "boballocator.h"
#include "bobobject.h"
#include <list>

using namespace std;


BobAllocator BobAllocator::the_allocator;


struct BobAllocator::AllocImpl
{
    list<BobObject*> live_objects;
    size_t total_alloc_size;

    AllocImpl()
        : total_alloc_size(0)
    {
    }
};


BobAllocator::BobAllocator()
    : d(new AllocImpl())
{
}
 

BobAllocator::~BobAllocator()
{
    delete d;
}


void* BobAllocator::allocate_object(size_t sz)
{
    void* mem = ::operator new(sz);
    d->live_objects.push_back(static_cast<BobObject*>(mem));
    d->total_alloc_size += sz;
    return mem;
}


void BobAllocator::release_object(void* p)
{
    ::operator delete(p);
}


