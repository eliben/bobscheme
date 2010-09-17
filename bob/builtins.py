#-------------------------------------------------------------------------------
# bob: builtins.py
#
# Some builtin Sheme procedures used by the interpreter and VM. 
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import operator
from expr import *


class BuiltinProcedure(object):
    """ A lightweight representation of builtin procedures, parallel to the
        approach taken in SICP.
        
        The calling convention for builtin procedures is as follows:
        
        Arguments are passed in as a Python list. Each argument is a Scheme
        expression (from the expr module). The procedure should always return
        a single value which is also a Scheme expression.
    """
    def __init__(self, name, proc):
        self.name = name
        self.proc = proc
    
    def apply(self, args):
        return self.proc(args)


class BuiltinError(Exception): pass


def builtin_pair_p(args):
    return Boolean(isinstance(args[0], Pair))

def builtin_boolean_p(args):
    return Boolean(isinstance(args[0], Boolean))

def builtin_symbol_p(args):
    return Boolean(isinstance(args[0], Symbol))

def builtin_number_p(args):
    return Boolean(isinstance(args[0], Number))

def builtin_zero_p(args):
    return Boolean(isinstance(args[0], Number) and args[0].value == 0)

def builtin_null_p(args):
    return Boolean(args[0] is None)

def builtin_cons(args):
    return Pair(args[0], args[1])

def builtin_list(args):
    return make_nested_pairs(*args)

def builtin_car(args):
    return args[0].first

def builtin_cdr(args):
    return args[0].second

def builtin_cadr(args):
    return args[0].second.first

def builtin_caddr(args):
    return args[0].second.second.first

def builtin_eqv(args):
    # A rough approximation of Scheme's eqv? that's good enough for most
    # practical purposes
    #
    left, right = args[0], args[1]
    
    if isinstance(left, Pair) and isinstance(right, Pair):
        return Boolean(id(left) == id(right))
    else:
        return Boolean(left == right)
    
def builtin_not(args):
    if isinstance(args[0], Boolean) and args[0].value == False:
        return Boolean(True)
    else:
        return Boolean(False)

def make_num_operator_builtin(opfunc, returntype):
    def op(args):
        left, right = args[0], args[1]
        
        if isinstance(left, Number) and isinstance(right, Number):
            return returntype(opfunc(left.value, right.value))
        else:
            raise BuiltinError('Builtin called with invalid types: %s & %s' % (type(left), type(right)))
    return op


builtins_map = {
    'eqv?':         builtin_eqv,
    'ev?':          builtin_eqv,
    'pair?':        builtin_pair_p,
    'zero?':        builtin_zero_p,
    'boolean?':     builtin_boolean_p,
    'symbol?':      builtin_symbol_p,
    'number?':      builtin_number_p,
    'null?':        builtin_null_p,
    'cons':         builtin_cons,
    'list':         builtin_list,
    'car':          builtin_car,
    'cdr':          builtin_cdr,
    'cadr':         builtin_cadr,
    'caddr':        builtin_caddr,
    'not':          builtin_not,
    '+':            make_num_operator_builtin(operator.add, Number),
    '-':            make_num_operator_builtin(operator.sub, Number),
    '*':            make_num_operator_builtin(operator.mul, Number),
    '=':            make_num_operator_builtin(operator.eq, Boolean),
    '>=':           make_num_operator_builtin(operator.ge, Boolean),
    '<=':           make_num_operator_builtin(operator.le, Boolean),
    '>':            make_num_operator_builtin(operator.gt, Boolean),
    '<':            make_num_operator_builtin(operator.lt, Boolean),
}                   
