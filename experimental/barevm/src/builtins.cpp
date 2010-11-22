//*****************************************************************************
// bob: Scheme builtin procedures
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "builtins.h"
#include "basicobjects.h"
#include "utils.h"
#include <cassert>

using namespace std;


static inline void builtin_verify(bool condition, const string& message)
{
    if (!condition)
        throw BuiltinError(message);
}


static inline void verify_numargs(BuiltinArgs args, size_t num, const string& name)
{
    builtin_verify(args.size() == num, format_string("%s expects %u arguments", name.c_str(), num));
}


static BobObject* car(BuiltinArgs args)
{
    verify_numargs(args, 1, "car");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "car expects a pair");
    return pair->first();
}


static BobObject* set_car(BuiltinArgs args)
{
    verify_numargs(args, 2, "set-car");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "set-car expects a pair as its first argument");
    pair->set_first(args[1]);
    return new BobNull();
}


// This one gets a longer name to avoid clash with a STL functor named
// logical_not
//
static BobObject* builtin_logical_not(BuiltinArgs args)
{
    verify_numargs(args, 1, "not");
    BobBoolean* val = dynamic_cast<BobBoolean*>(args[0]);
    builtin_verify(val, "not expects a boolean");
    return new BobBoolean(!val->value());
}


BuiltinsMap make_builtins_map()
{
    BuiltinsMap builtins_map;

    builtins_map["car"] = car;
    builtins_map["set-car"] = set_car;
    builtins_map["not"] = builtin_logical_not;

    return builtins_map;
}

