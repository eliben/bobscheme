#include "mem.h"
#include "object.h"


static BobObject* BobObject_alloc(BobObjectType type)
{
    BobObject* obj = mem_alloc(sizeof(*obj));
    obj->type = type;
    return obj;
}


BobObject* BobNull_new()
{
    BobObject* obj = BobObject_alloc(TYPE_NULL);
    return obj;
}


BobObject* BobNumber_new(int num)
{
    BobObject* obj = BobObject_alloc(TYPE_NUMBER);
    obj->d.num = num;
    return obj;
}


BobObject* BobSymbol_new(const char* sym)
{
    BobObject* obj = BobObject_alloc(TYPE_SYMBOL);
    obj->d.sym = sym;
    return obj;
}


BobObject* BobPair_new(BobObject* first, BobObject* second)
{
    BobObject* obj = BobObject_alloc(TYPE_PAIR);
    obj->d.pair.first = first;
    obj->d.pair.second = second;
    return obj;
}

