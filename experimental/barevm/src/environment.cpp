//*****************************************************************************
// bob: Environment object
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "environment.h"

using namespace std;


BobObject* BobEnvironment::lookup_var(const string& name)
{
    Binding::const_iterator it = binding.find(name);
    if (it == binding.end())
        return parent ? parent->lookup_var(name) : 0;
    else
        return it->second;
}


void BobEnvironment::define_var(const string& name, BobObject* value)
{
    binding[name] = value;
}


BobObject* BobEnvironment::set_var_value(const string& name, BobObject* value)
{
    Binding::const_iterator it = binding.find(name);
    if (it == binding.end())
        return parent ? parent->set_var_value(name, value) : 0;
    else {
        binding[name] = value;
        return value;
    }
}

