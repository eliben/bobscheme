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

    // Derived classes that want to be comparable should override this
    // method. 
    // It shall be invoked only with 'other' of the same class as the
    // object it's invoked on.
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
