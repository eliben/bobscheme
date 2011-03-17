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
#include <functional>
#include <iostream>
#include <numeric>
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


// Try to dynamically case arg to T* and return the cast pointer. On failure, 
// throw BuiltinError with message as the error.
//
template <class T>
static inline T* verify_argtype(BobObject* arg, string message)
{
    T* arg_t = dynamic_cast<T*>(arg);
    if (!arg_t)
        throw BuiltinError(message);
    return arg_t;
}


static BobObject* car(BuiltinArgs& args)
{
    verify_numargs(args, 1, "car");
    BobPair* pair = verify_argtype<BobPair>(args[0], "car expects a pair");
    return pair->first();
}


static BobObject* cdr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "cdr");
    BobPair* pair = verify_argtype<BobPair>(args[0], "cdr expects a pair");
    return pair->second();
}


static BobObject* cadr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "cadr");
    BobPair* pair = verify_argtype<BobPair>(args[0], "cadr expects a pair");
    BobPair* cdr_pair = verify_argtype<BobPair>(pair->second(), "cadr expects arg's cdr to be a pair");
    return cdr_pair->first();
}


static BobObject* caddr(BuiltinArgs& args)
{
    verify_numargs(args, 1, "caddr");
    BobPair* pair = verify_argtype<BobPair>(args[0], "caddr expects a pair");
    BobPair* cdr_pair = verify_argtype<BobPair>(pair->second(), "caddr expects arg's cdr to be a pair");
    BobPair* cddr_pair = verify_argtype<BobPair>(cdr_pair->second(), "caddr expects arg's cddr to be a pair");
    return cddr_pair->first();
}


static BobObject* set_car(BuiltinArgs& args)
{
    verify_numargs(args, 2, "set-car");
    BobPair* pair = verify_argtype<BobPair>(args[0], "set-car expects a pair");
    pair->set_first(args[1]);
    return new BobNull();
}


static BobObject* set_cdr(BuiltinArgs& args)
{
    verify_numargs(args, 2, "set-cdr");
    BobPair* pair = verify_argtype<BobPair>(args[0], "set-cdr expects a pair");
    pair->set_second(args[1]);
    return new BobNull();
}


static BobObject* cons(BuiltinArgs& args)
{
    verify_numargs(args, 2, "cons");
    //cerr << "==== Before GC In cons\n" << BobAllocator::get().stats_all_live() << endl;
    //cerr << "====== Running GC ======\n";
    //BobAllocator::get().run_gc();
    //cerr << "==== After GC In cons\n" << BobAllocator::get().stats_all_live() << endl;
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
    BobBoolean* val = verify_argtype<BobBoolean>(args[0], "not expects a boolean");
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


// A generic arithmetic builtin that's parametrized by a binary operation
// that must support being called as func(int, int) returning int.
//
template <class ArithmeticFunction>
static BobObject* builtin_arithmetic_generic(
                        string name,
                        BuiltinArgs& args, 
                        ArithmeticFunction func)
{
    string typeerrmsg = name + " expects a numeric argument";
    builtin_verify(args.size() > 0, name + " expects arguments");
    BobNumber* firstarg = verify_argtype<BobNumber>(args[0], typeerrmsg);
    int result = firstarg->value();
    for (BuiltinArgs::iterator arg = args.begin() + 1; arg != args.end(); ++arg) {
        BobNumber* argnum = verify_argtype<BobNumber>(*arg, typeerrmsg);
        result = func(result, argnum->value());
    }
    return new BobNumber(result);
}


static BobObject* builtin_add(BuiltinArgs& args)
{
    return builtin_arithmetic_generic("+", args, plus<int>());
}


static BobObject* builtin_sub(BuiltinArgs& args)
{
    return builtin_arithmetic_generic("-", args, minus<int>());
}


static BobObject* builtin_mul(BuiltinArgs& args)
{
    return builtin_arithmetic_generic("*", args, multiplies<int>());
}


static BobObject* builtin_quotient(BuiltinArgs& args)
{
    return builtin_arithmetic_generic("quotient", args, divides<int>());
}


static BobObject* builtin_modulo(BuiltinArgs& args)
{
    return builtin_arithmetic_generic("modulo", args, modulus<int>());
}


// A generic comparison builtin that's parametrized by a binary operation
// that must support being called as func(int, int) returning bool.
//
template <class ComparisonFunction>
static BobObject* builtin_comparison_generic(
                        string name,
                        BuiltinArgs& args,
                        ComparisonFunction func)
{
    string typeerrmsg = name + " expectes a numeric argument";
    builtin_verify(args.size() > 0, name + " expects arguments");
    BobNumber* a = verify_argtype<BobNumber>(args[0], typeerrmsg);
    for (BuiltinArgs::iterator arg = args.begin() + 1; arg != args.end(); ++arg) {
        BobNumber* b = verify_argtype<BobNumber>(*arg, typeerrmsg);
        if (func(a->value(), b->value()))
            a = b;
        else
            return new BobBoolean(false);
    }
    return new BobBoolean(true);
}


static BobObject* builtin_equal_to(BuiltinArgs& args)
{
    return builtin_comparison_generic("=", args, equal_to<int>());
}


static BobObject* builtin_greater_equal(BuiltinArgs& args)
{
    return builtin_comparison_generic(">=", args, greater_equal<int>());
}


static BobObject* builtin_less_equal(BuiltinArgs& args)
{
    return builtin_comparison_generic("<=", args, less_equal<int>());
}


static BobObject* builtin_greater(BuiltinArgs& args)
{
    return builtin_comparison_generic(">", args, greater<int>());
}


static BobObject* builtin_less(BuiltinArgs& args)
{
    return builtin_comparison_generic("<", args, less<int>());
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
    builtins_map["set-car!"] = set_car;
    builtins_map["set-cdr!"] = set_cdr;
    builtins_map["cons"] = cons;
    builtins_map["pair?"] = pair_p;
    builtins_map["number?"] = number_p;
    builtins_map["null?"] = null_p;
    builtins_map["boolean?"] = boolean_p;
    builtins_map["symbol?"] = symbol_p;
    builtins_map["zero?"] = zero_p;
    builtins_map["list"] = builtin_list;
    builtins_map["+"] = builtin_add;
    builtins_map["-"] = builtin_sub;
    builtins_map["*"] = builtin_mul;
    builtins_map["quotient"] = builtin_quotient;
    builtins_map["modulo"] = builtin_modulo;
    builtins_map["not"] = builtin_logical_not;
    builtins_map["or"] = builtin_logical_or;
    builtins_map["and"] = builtin_logical_and;
    builtins_map["="] = builtin_equal_to;
    builtins_map[">="] = builtin_greater_equal;
    builtins_map["<="] = builtin_less_equal;
    builtins_map[">"] = builtin_greater;
    builtins_map["<"] = builtin_less;

    return builtins_map;
}

