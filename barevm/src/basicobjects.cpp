//*****************************************************************************
// bob: Scheme basic objects
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "basicobjects.h"
#include "utils.h"
#include <typeinfo>
#include <cassert>

using namespace std;


// ----------- BobNull ------------
//
bool BobNull::equals_to(const BobObject& other) const 
{ 
    (void)other;
    // This will be called only for another BobNull, and two nulls are
    // always equal
    //
    return true;
}


string BobNull::repr() const
{
    return "()";
}


// ----------- BobBoolean ------------
//
bool BobBoolean::equals_to(const BobObject& other) const
{
    const BobBoolean& other_num = static_cast<const BobBoolean&>(other);
    return other_num.m_value == m_value;
}


string BobBoolean::repr() const
{
    return m_value ? "#t" : "#f";
}


// ----------- BobNumber ------------
//
bool BobNumber::equals_to(const BobObject& other) const
{
    const BobNumber& other_num = static_cast<const BobNumber&>(other);
    return other_num.m_value == m_value;
}


string BobNumber::repr() const
{
    return value_to_string(m_value);
}


// ----------- BobSymbol ------------
//
bool BobSymbol::equals_to(const BobObject& other) const 
{
    const BobSymbol& other_sym = static_cast<const BobSymbol&>(other);
    return other_sym.m_value == m_value;
}


string BobSymbol::repr() const
{
    return m_value;
}


// ----------- BobPair ------------
//
bool BobPair::equals_to(const BobObject& other) const 
{ 
    const BobPair& other_pair = static_cast<const BobPair&>(other);
    return (objects_equal(m_first, other_pair.m_first) &&
            objects_equal(m_second, other_pair.m_second));
}


string BobPair::repr() const
{
    string rep = "(" + repr_internal() + ")";
    return rep;
}


void BobPair::gc_mark_pointed()
{
    m_first->gc_mark();
    m_second->gc_mark();
}


std::string BobPair::repr_internal() const
{
    assert(m_first && "Expect valid pointer in m_first");
    string rep = m_first->repr();
    
    // Linearizes a nested pair structure. I.e:
    // Pair(1, Pair(2, Null)) ==> 1 2
    // The loop runs until the current pair's m_second is no longer a pair.
    //
    const BobPair* pair = this;
    while (const BobPair* second_pair = dynamic_cast<const BobPair*>(pair->m_second)) {
        assert(second_pair->m_first && "Expect valid pointer in m_first");
        rep += " " + second_pair->m_first->repr();
        pair = second_pair;
    }
    assert(pair->m_second && "Expect valid pointer in m_second");

    if (typeid(*pair->m_second) == typeid(BobNull))
        return rep;
    else
        return rep + " . " + pair->m_second->repr();
}

