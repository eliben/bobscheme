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


struct VMError : public std::runtime_error
{
    VMError(const std::string& reason)
        : std::runtime_error(reason)
    {}
};


struct VMData;

class BobVM 
{
public:
    BobVM(const std::string& output_file="");
    virtual ~BobVM();

private:
    VMData* d;
};


#endif /* VM_H */

