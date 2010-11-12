//*****************************************************************************
// bob: base BobObject
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include <typeinfo>
#include "bobobject.h"


bool objects_equal(const BobObject* lhs, const BobObject* rhs)
{
    if (lhs == rhs)
        return true;
    else if (typeid(*lhs) != typeid(*rhs))
        return false;
    else
        return lhs->equals_to(*rhs);
}

