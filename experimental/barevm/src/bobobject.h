//*****************************************************************************
// bob: base BobObject
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BOBOBJECT_H
#define BOBOBJECT_H

#include <string>


// Abstract base class for all objects managed by the Bob VM
//
class BobObject 
{
public:
    virtual ~BobObject() = 0;

    virtual std::string repr() const
    {
        return "<object>";
    }

    // Derived objects must override this comparison function. An object can
    // assume that 'other' is of the same type as it is.
    //
    virtual bool equals_to(const BobObject& other) const
    {
        return false;
    }
};


// Compare two objects of any type derived from BobObject
//
bool objects_equal(const BobObject*, const BobObject*);


#endif /* BOBOBJECT_H */
