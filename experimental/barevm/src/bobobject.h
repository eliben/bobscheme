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
    BobObject();
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
        (void)other;
        return false;
    }

    void* operator new(size_t sz);
    void operator delete(void* p);

    // Garbage collection code.
    //
    virtual void gc_mark()
    {
        m_gc_occupied = true;
        gc_mark_pointed();
    }

    virtual void gc_clear() 
    {
        m_gc_occupied = false;
    }

    // The default implementation does nothing here, to simplify the trivial
    // objects that hold no pointers to other objects
    //
    virtual void gc_mark_pointed()
    {
    }

    bool m_gc_occupied;
};


// Compare two objects of any type derived from BobObject
//
bool objects_equal(const BobObject*, const BobObject*);


#endif /* BOBOBJECT_H */
