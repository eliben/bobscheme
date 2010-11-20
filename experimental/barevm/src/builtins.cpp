//*****************************************************************************
// bob: Scheme builtin procedures
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "builtins.h"
#include "basicobjects.h"
#include <cassert>

using namespace std;


namespace bob_builtin {

static inline void builtin_verify(bool condition, const string& message)
{
    if (!condition)
        throw BuiltinError(message);
}


BobObject* car(BuiltinArgs args)
{
    builtin_verify(args.size() == 1, "car expects one argument");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "car expects a pair");
    return pair->first();
}


BobObject* set_car(BuiltinArgs args)
{
    builtin_verify(args.size() == 2, "set-car expects two arguments");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "set-car expects a pair as its first argument");
    pair->set_first(args[1]);
    return new BobNull();
}

} // namespace bob_builtin
