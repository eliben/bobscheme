/******************************************************************************
** bob: builtins - implementation of some built-in Scheme procedures.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "assert.h"
#include "builtins.h"
#include "object.h"


struct BobObject* builtin_car(size_t nargs, struct BobObject** args)
{
    assert(nargs == 1);
    assert(args[0]->type == TYPE_PAIR);
    return args[0]->d.pair.first;
}


struct BobObject* builtin_cdr(size_t nargs, struct BobObject** args)
{
    assert(nargs == 1);
    assert(args[0]->type == TYPE_PAIR);
    return args[0]->d.pair.second;
}


struct BobObject* builtin_add(size_t nargs, struct BobObject** args)
{
    int sum = 0;
    size_t i;
    assert(nargs >= 1);

    for (i = 0; i < nargs; ++i) {
        assert(args[i]->type == TYPE_NUMBER);
        sum += args[i]->d.num;
    }

    return BobNumber_new(sum);
}
