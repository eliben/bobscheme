#-------------------------------------------------------------------------------
# bob: expr.py
#
# Internal representation for parsed Scheme code. Scheme code consists of 
# expressions, which are recursively defined as either "atoms" (numbers, etc.) 
# or lists of expressions. Lists are represented in the conventional Scheme 
# notation of nested cons cells (Pair objects).
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------

#
# Classes for expression objects 
# Note: The empty list is in a class of its own in Scheme. Here we represent it
# by the None object.
#
class Pair(object):
    """ Represents a Scheme pair ("cons cell"). 'first' and 'second' are 
        publicly accessible attributes.
        
        Scheme idiomatic accessors can be implemented as follows, given that 
        p is a Pair:
        
        car     ==> p.first
        cdr     ==> p.second
        cadr    ==> p.second.first
        caadr   ==> p.second.first.first
        ... etc.
        
        Note that if any of these actions is semantically invalid, a Python 
        AttributeError will be raised. This is in accord with the Scheme 
        standard, which states that (car '()) and (cdr '()) are errors.
    """
    def __init__(self, first, second):
        self.first = first
        self.second = second
    
    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.first == other.first and self.second == other.second
        else:
            return False


class Number(object):
    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return str(self.value)

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.value == other.value
        else:
            return self.value == other


class Symbol(object):
    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return self.value
    
    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.value == other.value
        else:
            return self.value == other


class Boolean(object):
    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return '#t' if self.value else '#f'

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.value == other.value
        else:
            return False


# An exception that can be raised by the various functions in this module when
# there's an error with the Scheme expressions they're asked to process.
#
class ExprError(Exception): pass


def expr_repr(expr):
    """ A textual representation of the given Scheme expression.
    """
    def repr_rec(obj):
        if obj is None:
            return '()'
        elif isinstance(obj, (Boolean, Symbol, Number)):            
            return repr(obj)
        elif isinstance(obj, Pair):
            str = '(' + repr_rec(obj.first)
            while isinstance(obj.second, Pair): 
                str += ' ' + repr_rec(obj.second.first)
                obj = obj.second
            if obj.second is None:
                str += ')'
            else:
                str += ' . ' + repr_rec(obj.second) + ')'
            return str
        else:
            raise ExprError("Unexpected type: %s" % type(obj))

    return repr_rec(expr)


def make_nested_pairs(*args):
    """ Given a list of arguments, creates a list in Scheme representation 
        (nested Pairs)
    """
    if len(args) == 0:
        return None
    return Pair(args[0], make_nested_pairs(*args[1:]))


def expand_nested_pairs(pair, recursive=False):
    """ Given a list in Scheme representation (nested Pairs), expands it into
        a Python list.
        
        When recursive=True, expands nested pairs as well. I.e Scheme's 
        (1 (2 3) 4) is correctly translated to [1, [2, 3], 4]). 
        Ignores dotted-pair endings: (1 2 . 3) will be translated to [1, 2]
    """
    lst = []
    while isinstance(pair, Pair):
        head = pair.first
        if recursive and isinstance(head, Pair):
            lst.append(expand_nested_pairs(head))
        else:
            lst.append(head)
        pair = pair.second
    return lst


def is_scheme_expr(exp):
    """ Check if the given expression is a Scheme expression.
    """
    return exp is None or is_self_evaluating(exp) or is_variable(exp) or isinstance(exp, Pair)


#
# Dissection of Scheme expressions into their constituents. Roughly follows 
# section 4.1.2 of SICP.
#
def is_self_evaluating(exp):
    return isinstance(exp, (Number, Boolean))

def is_variable(exp):
    return isinstance(exp, Symbol)

def is_tagged_list(exp, tag):
    """ Is the expression a list starting with the given symbolic tag?
    """
    return isinstance(exp, Pair) and exp.first == tag


def is_quoted(exp):
    return is_tagged_list(exp, 'quote')

def text_of_quotation(exp):
    return exp.second.first


def is_assignment(exp):
    return is_tagged_list(exp, 'set!')

def assignment_variable(exp):
    return exp.second.first

def assignment_value(exp):
    return exp.second.second.first


#
# Definitions have the form
#   (define <var> <value>)
# or the form
#   (define (<var> <parameter1> ... <parametern>)
#     <body>)
#
# The latter form (standard procedure definition) is syntactic sugar for
#
#   (define <var>
#     (lambda (<parameter1> ... <parametern>)
#       <body>))
#
def is_definition(exp):
    return is_tagged_list(exp, 'define')

def definition_variable(exp):
    if isinstance(exp.second.first, Symbol):
        return exp.second.first
    else:
        return exp.second.first.first

def definition_value(exp):
    if isinstance(exp.second.first, Symbol):
        return exp.second.second.first
    else:
        return make_lambda(
                    exp.second.first.second,    # formal parameters
                    exp.second.second)          # body


def is_lambda(exp):
    return is_tagged_list(exp, 'lambda')

def lambda_parameters(exp):
    return exp.second.first

def lambda_body(exp):
    return exp.second.second

def make_lambda(parameters, body):
    return Pair(Symbol('lambda'), Pair(parameters, body))


def is_if(exp):
    return is_tagged_list(exp, 'if')

def if_predicate(exp):
    return exp.second.first

def if_consequent(exp):
    return exp.second.second.first

def if_alternative(exp):
    alter_exp = exp.second.second.second
    if alter_exp is None:
        return Boolean(False)
    else:
        return alter_exp.first
  
def make_if(predicate, consequent, alternative):
    return make_nested_pairs(Symbol('if'), predicate, consequent, alternative)
    

def is_begin(exp):
    return is_tagged_list(exp, 'begin')

def begin_actions(exp):
    return exp.second

def is_last_exp(seq):
    return seq.second is None

def first_exp(seq):
    return seq.first

def rest_exps(seq):
    return seq.second


#
# Procedure applications
#
def is_application(exp):
    return isinstance(exp, Pair)

def application_operator(exp):
    return exp.first

def application_operands(exp):
    return exp.second

def has_no_operands(ops):
    return ops is None

def first_operand(ops):
    return ops.first

def rest_operands(ops):
    return ops.second


def sequence_to_exp(seq):
    """ Convert a sequence of expressions to a single expression, adding 'begin
        if required.
    """
    if seq is None:
        return None
    elif is_last_exp(seq):
        return first_exp(seq)
    else:
        return Pair(Symbol('begin'), seq)
    

#
# 'cond' is a derived expression and is expanded into a series of nested 'if's.
#
def is_cond(exp):
    return is_tagged_list(exp, 'cond')

def cond_clauses(exp):
    return exp.second

def cond_predicate(clause):
    return clause.first

def cond_actions(clause):
    return clause.second

def is_cond_else_clause(clause):
    return cond_predicate(clause) == Symbol('else')

def convert_cond_to_ifs(exp):
    return expand_cond_clauses(cond_clauses(exp))

def expand_cond_clauses(clauses):
    if clauses is None:
        return Boolean(False)
    first = clauses.first
    rest = clauses.second
    if is_cond_else_clause(first):
        if rest is None:
            return sequence_to_exp(cond_actions(first))
        else:
            raise ExprError('ELSE clause is not last: %s' % expr_repr(clauses))
    else:
        return make_if(
                    predicate=cond_predicate(first), 
                    consequent=sequence_to_exp(cond_actions(first)),
                    alternative=expand_cond_clauses(rest))


#
# 'let' is a derived expression:
#
# (let ((var1 exp1) ... (varN expN))
#     body)
#
# is expanded to:
#
# ((lambda (var1 ... varN)
#     body)
#   exp1
#   ...
#   expN)
#
def is_let(exp):
    return is_tagged_list(exp, 'let')

def let_bindings(exp):
    return exp.second.first

def let_body(exp):
    return exp.second.second

def convert_let_to_application(exp):
    """ Given a Scheme 'let' expression converts it to the appropriate 
        application of an anonymous procedure.
    """
    # Extract lists of var names and values from the bindings of 'let'.
    # bindings is a (Scheme) list of 2-element (var val) lists.
    #
    vars = []
    vals = []
    
    bindings = let_bindings(exp)
    while bindings is not None:
        vars.append(bindings.first.first)
        vals.append(bindings.first.second.first)
        bindings = bindings.second
    
    lambda_expr = make_lambda(make_nested_pairs(*vars), let_body(exp))
    return make_nested_pairs(lambda_expr, *vals)


#------------------------------------------------------------------------------
if __name__ == '__main__':
    pass
    
