//*****************************************************************************
// bob: The virtual machine implementation
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef VM_H
#define VM_H

#include "bytecode.h"
#include <string>
#include <stdexcept>


// The exception raised by BobVM for execution errors.
//
struct VMError : public std::runtime_error
{
    VMError(const std::string& reason)
        : std::runtime_error(reason)
    {}
};


struct VMImpl;

// Implementation of the Bob VM.
// Usage: create, then .run() with a code object.
// The only side effect it produces currently is to print output to the given 
// file. If output_file is "", prints to stdout. 
// 
class BobVM 
{
public:
    BobVM(const std::string& output_file="");
    virtual ~BobVM();

    void run(BobCodeObject* codeobj);
    void set_gc_size_threshold(std::size_t threshold); 
private:
    friend class BobAllocator;
    BobVM(const BobVM&);
    BobVM& operator=(const BobVM&);

    // Run gc_mark on all root objects the VM holds. This method 
    // is called by the garbage collector.
    //
    void gc_mark_roots();

    VMImpl* d;
};


#endif /* VM_H */

