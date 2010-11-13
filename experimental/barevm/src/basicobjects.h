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

    ~BobNull()
    {}

    std::string repr() const;
    bool equals_to(const BobObject& other) const;
};


class BobBoolean : public BobObject
{
public:
    BobBoolean(bool value)
        : m_value(value)
    {}

    ~BobBoolean()
    {}

    std::string repr() const;
    bool equals_to(const BobObject& other) const;

private:
    bool m_value;
};


class BobNumber : public BobObject
{
public:
    BobNumber(int value)
        : m_value(value)
    {}

    ~BobNumber()
    {}

    std::string repr() const;
    bool equals_to(const BobObject& other) const;
private:
    int m_value;
};


class BobSymbol : public BobObject
{
public:
    BobSymbol(const std::string& value)
        : m_value(value)
    {}

    ~BobSymbol()
    {}

    std::string repr() const;
    bool equals_to(const BobObject& other) const;
private:
    std::string m_value;
};


class BobPair : public BobObject
{
public:
    BobPair(const BobObject* first, const BobObject* second)
        : m_first(first), m_second(second)
    {}

    ~BobPair()
    {}

    inline const BobObject* get_first() const;
    inline const BobObject* get_second() const;
    std::string repr() const;
    bool equals_to(const BobObject& other) const;
private:
    std::string repr_internal() const;

    const BobObject* m_first;
    const BobObject* m_second;
};

#endif /* BASICOBJECTS_H */

