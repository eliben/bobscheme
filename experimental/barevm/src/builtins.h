//*****************************************************************************
// bob: Scheme builtin procedures
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BUILTINS_H
#define BUILTINS_H

#include "bobobject.h"
#include "utils.h"
#include <string>
#include <map>
#include <vector>
#include <stdexcept>


// Builtin procedures are implemented as normal C++ functions with the 
// following signature. They accept a varying amount of arguments in 
// a vector and return a single value. All values (args and return value)
// are BobObjects.
//
typedef std::vector<BobObject*> BuiltinArgs;
typedef std::vector<BobObject*>::iterator BuiltinArgsIterator;
typedef std::vector<BobObject*>::const_iterator BuiltinArgsIteratorConst;
typedef BobObject* (*BuiltinProc)(BuiltinArgs& args);


// The exception type thrown by builtins when they're used incorrectly by
// the user. Note: this is reserved for errors that *the user can make*,
// like calling (car 2) [car must have a pair as its argument].
//
struct BuiltinError : public std::runtime_error
{
    BuiltinError(const std::string& reason)
        : std::runtime_error(reason)
    {}
};
            

// A simple wrapper for a builtin procedure. Derived from BobObject for 
// convenience (this way it can be kept in an environment bound to a name of 
// a builtin).
//
class BobBuiltinProcedure : public BobObject
{
public:
    BobBuiltinProcedure(const std::string& name, BuiltinProc proc)
        : m_name(name), m_proc(proc)
    {}

    virtual ~BobBuiltinProcedure()
    {}

    virtual const std::string& name() const
    {
        return m_name;
    }

    virtual BobObject* exec(BuiltinArgs& args) const
    {
        return m_proc(args);
    }

    virtual std::string repr() const
    {
        return format_string("<builtin '%s'>", m_name.c_str());
    }
private:
    std::string m_name;
    BuiltinProc m_proc;
};


// Call init_builtins_map to fill in a BuiltinsMap with all the available
// builtins.
//
typedef std::map<std::string, BuiltinProc> BuiltinsMap;
BuiltinsMap make_builtins_map();


#endif /* BUILTINS_H */

