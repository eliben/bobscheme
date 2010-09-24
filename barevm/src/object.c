/******************************************************************************
** bob: Scheme objects for bare VM
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "mem.h"
#include "object.h"
#include "objectallocator.h"
#include "intern.h"


static BobObject* BobObject_new(BobObjectType type)
{
    BobObject* obj = BobObject_alloc();
    obj->type = type;
    return obj;
}


BobObject* BobNull_new()
{
    BobObject* obj = BobObject_new(TYPE_NULL);
    return obj;
}


BobObject* BobNumber_new(int num)
{
    BobObject* obj = BobObject_new(TYPE_NUMBER);
    obj->d.num = num;
    return obj;
}


BobObject* BobSymbol_new(dstring sym)
{
    BobObject* obj = BobObject_new(TYPE_SYMBOL);
    obj->d.sym = intern_dstring(sym);
    return obj;
}


BobObject* BobPair_new(BobObject* first, BobObject* second)
{
    BobObject* obj = BobObject_new(TYPE_PAIR);
    obj->d.pair.first = first;
    obj->d.pair.second = second;
    return obj;
}

