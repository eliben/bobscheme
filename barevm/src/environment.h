//*****************************************************************************
// bob: Environment object
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "bobobject.h"
#include <string>
#include <map>


// An environment in which variables are bound to values. Variable names are 
// strings, values are BobObject*.
// 
// Environment objects are linked via parent pointers. When bindings are 
// queried or assigned and the variable name isn't bound in the environment,
// its parent is searched.
//
// An environment hierarchy ultimately terminates with a "top-level"
// environment, which has 0 as its parent link.
//
class BobEnvironment : public BobObject
{
public:
    // Create a new, empty environment with the given parent link
    //
    BobEnvironment(BobEnvironment* parent=0)
        : m_parent(parent)
    {}

    // Lookup the variable in this environment or its parents. Return the 
    // object if found, 0 otherwise.
    //
    BobObject* lookup_var(const std::string& name);

    // Add a name -> value binding to this environment. If a binding for the
    // name already exists, it is overridden.
    //
    void define_var(const std::string& name, BobObject* value);

    // Find the binding of name in this environment or its parents and assign
    // the new value to it. Return the value if successful, or 0 if no 
    // binding for the name was found.
    //
    BobObject* set_var_value(const std::string& name, BobObject* value);

    virtual ~BobEnvironment()
    {}

    virtual void gc_mark_pointed();
private:
    BobEnvironment* m_parent;
    typedef std::map<std::string, BobObject*> Binding;
    Binding m_binding;
};



#endif /* ENVIRONMENT_H */
