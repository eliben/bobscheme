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
#include <map>
#include <vector>
#include <stdexcept>


// Builtin procedures are implemented as normal C++ functions with the 
// following signature. They accept a varying amount of arguments in 
// a vector and return a single value. All values (args and return value)
// are BobObjects.
//
typedef const std::vector<BobObject*>& BuiltinArgs;
typedef BobObject* (*BuiltinProc)(BuiltinArgs args);


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
// a builtin.
//
class BobBuiltinProcedure : public BobObject
{
public:
    BobBuiltinProcedure(const std::string& name, BuiltinProc proc)
        : m_name(name), m_proc(proc)
    {}

    virtual ~BobBuiltinProcedure()
    {}

    const std::string& name() const
    {
        return m_name;
    }

    BuiltinProc proc() const
    {
        return m_proc;
    }
private:
    std::string m_name;
    BuiltinProc m_proc;
};


// BuiltinsMap is a mapping of names to procedures. 
// Call init_builtins_map to fill in a BuiltinsMap with all the available
// builtins.
//
typedef std::map<std::string, BobBuiltinProcedure*> BuiltinsMap;
void init_builtins_map(BuiltinsMap& map);

// Declarations of builtins. They live in a namespace since I want to use
// simple Scheme-like names, and yet not pollute the global namespace.
//
namespace bob_builtin {

#define DECLARE_BUILTIN(name) BobObject* name(BuiltinArgs args);

DECLARE_BUILTIN(car)
DECLARE_BUILTIN(set_car)
DECLARE_BUILTIN(logical_not)

} // namespace bob_builtin


#endif /* BUILTINS_H */

