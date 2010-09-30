/******************************************************************************
** bob: Scheme objects for bare VM
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <assert.h>
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


BobObject* BobBoolean_new(BOOL boolval)
{
    BobObject* obj = BobObject_new(TYPE_BOOLEAN);
    obj->d.boolval = boolval;
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


BobObject* BobPort_new(FILE* file)
{
    BobObject* obj = BobObject_new(TYPE_PORT);
    obj->d.port.file = file;
    return obj;
}


BobObject* BobPair_new(BobObject* first, BobObject* second)
{
    BobObject* obj = BobObject_new(TYPE_PAIR);
    obj->d.pair.first = first;
    obj->d.pair.second = second;
    return obj;
}


BobObject* BobBuiltin_new(dstring name, builtin_proc_type proc)
{
    BobObject* obj = BobObject_new(TYPE_BUILTIN_PROC);
    obj->d.builtin.proc = proc;
    obj->d.builtin.name = name;
    return obj;
}


BobObject* BobClosure_new(struct BobCodeObject* codeobj, struct BobEnv* env)
{
    BobObject* obj = BobObject_new(TYPE_CLOSURE);
    obj->d.closure.codeobj = codeobj;
    obj->d.closure.env = env;
    return obj;
}


void BobObject_repr(BobObject* obj, dstring repr)
{
    switch (obj->type) {
        case TYPE_NULL:
            dstring_concat_cstr(repr, "()");
            break;
        case TYPE_BOOLEAN:
            if (obj->d.boolval == TRUE)
                dstring_concat_cstr(repr, "#t");
            else
                dstring_concat_cstr(repr, "#f");
            break;
        case TYPE_NUMBER:
            {
                dstring numstr = dstring_format("%d", obj->d.num);
                dstring_concat(repr, numstr);
                dstring_free(numstr);
                break;
            }
        case TYPE_SYMBOL:
            dstring_concat(repr, obj->d.sym);
            break;
        case TYPE_PAIR:
            {
                dstring_concat_cstr(repr, "(");
                BobObject_repr(obj->d.pair.first, repr);
                while (obj->d.pair.second->type == TYPE_PAIR)  {
                    dstring_concat_cstr(repr, " ");
                    BobObject_repr(obj->d.pair.second->d.pair.first, repr);
                    obj = obj->d.pair.second;
                }
                if (obj->d.pair.second->type == TYPE_NULL) {
                    dstring_concat_cstr(repr, ")");
                }
                else {
                    dstring_concat_cstr(repr, " . ");
                    BobObject_repr(obj->d.pair.second, repr);
                    dstring_concat_cstr(repr, ")");
                }
                break;
            }
        default:
            assert(0 && "BobObject_repr: Unexpected type of BobObject*");
    }
}
