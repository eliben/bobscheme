//*****************************************************************************
// bob: Scheme basic objects
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BASICOBJECTS_H
#define BASICOBJECTS_H

#include <string>
#include "bobobject.h"


class BobNull : public BobObject
{
public:
    BobNull()
    {}

protected:
    bool equals_to(const BobObject& other) const
    {
        // This will be called only for another BobNull, and two nulls are
        // always equal
        //
        return true;
    }
};


class BobNumber : public BobObject
{
public:
    BobNumber(int value)
        : m_value(value)
    {}

protected:
    bool equals_to(const BobObject& other) const
    {
        const BobNumber& other_num = static_cast<const BobNumber&>(other);
        return other_num.m_value == m_value;
    }
private:
    int m_value;
};
#endif /* BASICOBJECTS_H */

