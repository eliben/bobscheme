//*****************************************************************************
// bob: Bob object allocator & garbage collector
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BOBALLOCATOR_H
#define BOBALLOCATOR_H

#include <cstdlib>
#include <string>


class BobAllocator 
{
public:
    static BobAllocator& get()
    {
        return the_allocator;
    }

    void* allocate_object(std::size_t sz);
    void release_object(void* p);

    std::string stats_general() const;
    std::string stats_all_live() const;

private:
    static BobAllocator the_allocator;

    BobAllocator(const BobAllocator&);
    BobAllocator& operator=(const BobAllocator&);
    BobAllocator();
    ~BobAllocator();

    struct AllocImpl;
    AllocImpl* d;
};

#endif /* BOBALLOCATOR_H */

