//*****************************************************************************
// bob: Scheme builtin procedures
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BUILTINS_H
#define BUILTINS_H

#include "bobobject.h"
#include <string>
#include <vector>


// Builtin procedures are implemented as normal C++ functions with the 
// following signature. They accept a varying amount of arguments in 
// a vector and return a single value. All values (args and return value)
// are BobObjects.
//
typedef BobObject* (*BuiltinProc)(const std::vector<BobObject*>& args);


// A simple wrapper for a builtin procedure. Derived from BobObject for 
// convenience (this way it can be kept in an environment bound to a name of 
// a builtin.
//
class BobBuiltinProcedure : public BobObject
{
public:
    BobBuiltinProcedure(const std::string& name_, BuiltinProc proc_)
        : name(name_), proc(proc_)
    {}

    virtual ~BobBuiltinProcedure()
    {}

    const std::string& get_name() const
    {
        return name;
    }

    BuiltinProc get_proc() const
    {
        return proc;
    }
private:
    std::string name;
    BuiltinProc proc;
};

#endif /* BUILTINS_H */

