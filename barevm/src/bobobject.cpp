//*****************************************************************************
// bob: base BobObject
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "bobobject.h"
#include "builtins.h"
#include "utils.h"
#include "vm.h"
#include <typeinfo>
#include <cstdlib>
#include <iostream>
#include <utility>

using namespace std;


BobObject::BobObject()
    : m_gc_marked(false)
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


// Singleton object definition
//
BobAllocator BobAllocator::the_allocator;


// The implementation details of the allocator
//
typedef pair<BobObject*, size_t> LiveObject;
struct BobAllocator::Impl 
{
    list<LiveObject> live_objects;
    size_t total_alloc_size;
    bool debug_on;

    BobVM* vm_obj;

    Impl()
        : total_alloc_size(0), debug_on(false), vm_obj(0)
    {
    }
};


BobAllocator::BobAllocator()
    : d(new BobAllocator::Impl)
{
}


BobAllocator::~BobAllocator()
{
    delete d;
}


void* BobAllocator::allocate_object(size_t sz)
{
    void* mem = ::operator new(sz);
    LiveObject liveobject = make_pair(static_cast<BobObject*>(mem), sz);
    d->live_objects.push_back(liveobject);
    d->total_alloc_size += sz;
    return mem;
}


void BobAllocator::release_object(void* p)
{
    ::operator delete(p);
}


void BobAllocator::register_vm_obj(BobVM* vm_obj)
{
    d->vm_obj = vm_obj;
}


void BobAllocator::set_debugging(bool debug_on)
{
    d->debug_on = debug_on;
}


string BobAllocator::stats_general() const
{
    string s = "========================================\n";
    s += format_string("Number of live objects: %u\n", d->live_objects.size());
    s += format_string("Total allocation size: %u\n", d->total_alloc_size);
    return s;
}


string BobAllocator::stats_all_live() const
{
    string s = "==== Live objects ====\n";
    for (list<LiveObject>::const_iterator it = d->live_objects.begin(); 
            it != d->live_objects.end(); ++it) {
        BobObject* obj = it->first;
        size_t size = it->second;
        if (dynamic_cast<BobBuiltinProcedure*>(obj) == 0)
            s += format_string("%s(%u) %s\n", 
                    typeid(*obj).name(), size, obj->repr().c_str());
    }
    return s;
}


void BobAllocator::run_gc(size_t size_threshold)
{
    if (d->total_alloc_size <= size_threshold)
        return;

    size_t old_num_live_objects = d->live_objects.size();
    size_t old_total_alloc_size = d->total_alloc_size;

    // Mark each object found in the roots. Marking as implemented by
    // BobObjectis subclasses is recursive.
    // Go over all the live objects:
    //   * Marked objects are used and thus have to keep living. Clear their
    //     mark flag.
    //   * Unmarked objects aren't used and can be deleted.
    d->vm_obj->gc_mark_roots();

    list<LiveObject>::iterator it = d->live_objects.begin();
    while (it != d->live_objects.end()) {
        BobObject* obj = it->first;
        size_t size = it->second;
        if (obj->is_gc_marked()) {
            obj->gc_clear();
            ++it;
        }
        else {
            it = d->live_objects.erase(it);
            delete obj; // garbage!!
            d->total_alloc_size -= size;
        }
    }

    // Debugging...
    if (d->debug_on && d->total_alloc_size != old_total_alloc_size) {
        cerr << "=== GC collection\n";
        cerr << format_string("--> was %u objects (total size %u)\n", 
                    old_num_live_objects, old_total_alloc_size);
        cerr << format_string("--> now %u objects (total size %u)\n",
                    d->live_objects.size(), d->total_alloc_size);
    }
}

