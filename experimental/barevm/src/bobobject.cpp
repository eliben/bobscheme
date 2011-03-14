//*****************************************************************************
// bob: base BobObject
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "bobobject.h"
#include "boballocator.h"
#include <typeinfo>
#include <cstdlib>

using namespace std;


BobObject::BobObject()
    : m_gc_occupied(false)
{
}


BobObject::~BobObject()
{
}


bool objects_equal(const BobObject* lhs, const BobObject* rhs)
{
    if (lhs == rhs)
        return true;
    else if (typeid(*lhs) != typeid(*rhs))
        return false;
    else
        return lhs->equals_to(*rhs);
}


void* BobObject::operator new(size_t sz)
{
    return BobAllocator::get().allocate_object(sz);
}


void BobObject::operator delete(void* p)
{
    BobAllocator::get().release_object(p);
}

