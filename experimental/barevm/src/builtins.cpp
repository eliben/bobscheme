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
#include <iterator>

using namespace std;


static inline void builtin_verify(bool condition, const string& message)
{
    if (!condition)
        throw BuiltinError(message);
}


static inline void verify_numargs(BuiltinArgs& args, size_t num, const string& name)
{
    builtin_verify(args.size() == num, format_string("%s expects %u arguments", name.c_str(), num));
}


static BobObject* car(BuiltinArgs& args)
{
    verify_numargs(args, 1, "car");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "car expects a pair");
    return pair->first();
}


static BobObject* cdr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "cdr");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "cdr expects a pair");
    return pair->second();
}


static BobObject* cadr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "cadr");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "cadr expects a pair");
    BobPair* cdr_pair = dynamic_cast<BobPair*>(pair->second());
    builtin_verify(cdr_pair, "cadr expects arg's cdr to be a pair");
    return cdr_pair->first();
}


static BobObject* caddr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "caddr");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "caddr expects a pair");
    BobPair* cdr_pair = dynamic_cast<BobPair*>(pair->second());
    builtin_verify(cdr_pair, "caddr expects arg's cdr to be a pair");
    BobPair* cddr_pair = dynamic_cast<BobPair*>(pair->second());
    builtin_verify(cddr_pair, "caddr expects arg's cddr to be a pair");
    return cddr_pair->first();
}


static BobObject* set_car(BuiltinArgs& args)
{
    verify_numargs(args, 2, "set-car");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "set-car expects a pair as its first argument");
    pair->set_first(args[1]);
    return new BobNull();
}


static BobObject* set_cdr(BuiltinArgs& args)
{
    verify_numargs(args, 2, "set-cdr");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    builtin_verify(pair, "set-cdr expects a pair as its first argument");
    pair->set_second(args[1]);
    return new BobNull();
}


static BobObject* cons(BuiltinArgs& args)
{
    verify_numargs(args, 2, "cons");
    return new BobPair(args[0], args[1]);
}


static BobObject* builtin_list(BuiltinArgs& args)
{
    BobObject* lst = new BobNull();
    vector<BobObject*> vv;

    typedef reverse_iterator<BuiltinArgsIterator> BuiltinArgsRevereIterator;
    BuiltinArgsRevereIterator rev_begin(args.end());
    BuiltinArgsRevereIterator rev_end(args.begin());

    for (BuiltinArgsRevereIterator i = rev_begin; i != rev_end; ++i) {
        lst = new BobPair(*i, lst);
    }

    return lst;
}


static BobObject* pair_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "pair?");
    BobPair* pair = dynamic_cast<BobPair*>(args[0]);
    return new BobBoolean(pair != 0);
}


static BobObject* boolean_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "boolean?");
    BobBoolean* boolean = dynamic_cast<BobBoolean*>(args[0]);
    return new BobBoolean(boolean != 0);
}


static BobObject* symbol_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "symbol?");
    BobSymbol* sym = dynamic_cast<BobSymbol*>(args[0]);
    return new BobBoolean(sym != 0);
}


static BobObject* number_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "number?");
    BobNumber* num = dynamic_cast<BobNumber*>(args[0]);
    return new BobBoolean(num != 0);
}


static BobObject* null_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "null?");
    BobNull* null = dynamic_cast<BobNull*>(args[0]);
    return new BobBoolean(null != 0);
}


static BobObject* zero_p(BuiltinArgs& args)
{
    verify_numargs(args, 1, "zero?");
    BobNumber* num = dynamic_cast<BobNumber*>(args[0]);
    return new BobBoolean(num && num->value() == 0);
}


static BobObject* builtin_logical_not(BuiltinArgs& args)
{
    verify_numargs(args, 1, "not");
    BobBoolean* val = dynamic_cast<BobBoolean*>(args[0]);
    builtin_verify(val, "not expects a boolean");
    return new BobBoolean(!val->value());
}


// The 'and' and 'or' builtins are conforming to the definition in R5RS,
// section 4.2
//
static BobObject* builtin_logical_or(BuiltinArgs& args)
{
    if (args.size() < 1)
        return new BobBoolean(false);
    
    for (BuiltinArgs::iterator arg = args.begin(); arg != args.end(); ++arg) {
        BobBoolean* boolval = dynamic_cast<BobBoolean*>(*arg);
        if (boolval && boolval->value())
            return boolval;
    }

    return args[args.size() - 1];
}


static BobObject* builtin_logical_and(BuiltinArgs& args)
{
    if (args.size() < 1)
        return new BobBoolean(true);
    
    for (BuiltinArgs::iterator arg = args.begin(); arg != args.end(); ++arg) {
        BobBoolean* boolval = dynamic_cast<BobBoolean*>(*arg);
        if (boolval && !boolval->value())
            return boolval;
    }

    return args[args.size() - 1];
}


// A rough approximation of Scheme's eqv? that's good enough for most purposes.
//
static BobObject* eqv_p(BuiltinArgs& args)
{
    verify_numargs(args, 2, "eqv?");
    BobObject* lhs = args[0];
    BobObject* rhs = args[1];

    if (dynamic_cast<BobPair*>(lhs) && dynamic_cast<BobPair*>(rhs))
        return new BobBoolean(lhs == rhs); // pointer comparison
    else
        return new BobBoolean(objects_equal(args[0], args[1]));
}


static BobObject* builtin_add(BuiltinArgs& args)
{
    int result = 0;
    for (BuiltinArgs::iterator arg = args.begin(); arg != args.end(); ++arg) {
        BobNumber* argnum = dynamic_cast<BobNumber*>(*arg);
        builtin_verify(argnum, "+ expects a numeric argument");
        result = result + argnum->value();
    }
    return new BobNumber(result);
}


BuiltinsMap make_builtins_map()
{
    BuiltinsMap builtins_map;

    builtins_map["eq?"] = eqv_p;
    builtins_map["eqv?"] = eqv_p;
    builtins_map["car"] = car;
    builtins_map["cdr"] = cdr;
    builtins_map["cadr"] = cadr;
    builtins_map["caddr"] = caddr;
    builtins_map["set-car"] = set_car;
    builtins_map["set-cdr"] = set_cdr;
    builtins_map["cons"] = cons;
    builtins_map["not"] = builtin_logical_not;
    builtins_map["or"] = builtin_logical_or;
    builtins_map["and"] = builtin_logical_and;
    builtins_map["pair?"] = pair_p;
    builtins_map["number?"] = number_p;
    builtins_map["null?"] = null_p;
    builtins_map["boolean?"] = boolean_p;
    builtins_map["symbol?"] = symbol_p;
    builtins_map["zero?"] = zero_p;
    builtins_map["list"] = builtin_list;
    builtins_map["+"] = builtin_add;

    return builtins_map;
}

