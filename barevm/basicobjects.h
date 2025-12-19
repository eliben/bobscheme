//*****************************************************************************
// bob: Scheme basic objects
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BASICOBJECTS_H
#define BASICOBJECTS_H

#include "bobobject.h"
#include <string>


// A Scheme "null" - empty list
//
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


// A Scheme boolean - true or false
//
class BobBoolean : public BobObject
{
public:
    BobBoolean(bool value)
        : m_value(value)
    {}

    ~BobBoolean()
    {}

    bool value() const {return m_value;}
    std::string repr() const;
    bool equals_to(const BobObject& other) const;

private:
    bool m_value;
};


// A Scheme number - integer
//
class BobNumber : public BobObject
{
public:
    BobNumber(int value)
        : m_value(value)
    {}

    ~BobNumber()
    {}

    int value() const {return m_value;}
    std::string repr() const;
    bool equals_to(const BobObject& other) const;
private:
    int m_value;
};


// A Scheme symbol - a constant string
//
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


// A Scheme pair - holds sub-objects 'first' and 'second'
//
class BobPair : public BobObject
{
public:
    BobPair(BobObject* first, BobObject* second)
        : m_first(first), m_second(second)
    {}

    ~BobPair()
    {}

    BobObject* first() {return m_first;}
    BobObject* second() {return m_second;}
    void set_first(BobObject* first) {m_first = first;}
    void set_second(BobObject* second) {m_second = second;}

    std::string repr() const;
    bool equals_to(const BobObject& other) const;

    virtual void gc_mark_pointed();

private:
    std::string repr_internal() const;

    BobObject* m_first;
    BobObject* m_second;
};

#endif /* BASICOBJECTS_H */

