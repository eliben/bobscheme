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
    Binding::const_iterator it = m_binding.find(name);
    if (it == m_binding.end())
        return m_parent ? m_parent->lookup_var(name) : 0;
    else
        return it->second;
}


void BobEnvironment::define_var(const string& name, BobObject* value)
{
    m_binding[name] = value;
}


BobObject* BobEnvironment::set_var_value(const string& name, BobObject* value)
{
    Binding::const_iterator it = m_binding.find(name);
    if (it == m_binding.end())
        return m_parent ? m_parent->set_var_value(name, value) : 0;
    else {
        m_binding[name] = value;
        return value;
    }
}
 

void BobEnvironment::gc_mark_pointed()
{
    for (Binding::iterator it = m_binding.begin(); it != m_binding.end(); ++it)
        it->second->gc_mark();
    if (m_parent)
        m_parent->gc_mark();
}


