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
#include <memory>


struct VMError : public std::runtime_error
{
    VMError(const std::string& reason)
        : std::runtime_error(reason)
    {}
};


struct VMImpl;

class BobVM 
{
public:
    BobVM(const std::string& output_file="");
    virtual ~BobVM();

private:
    BobVM(const BobVM&);
    BobVM& operator=(const BobVM&);

    std::auto_ptr<VMImpl> d;
};


#endif /* VM_H */

