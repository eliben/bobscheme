#-------------------------------------------------------------------------------
# bob: interpreter.py
#
# Scheme interpreter. 
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
import sys
import pprint

from .bobparser import BobParser
from .builtins import BuiltinProcedure, builtins_map
from .expr import *
from .environment import Environment


DEBUG = False


class Procedure(object):
    """ Represents a compound procedure (closure).
    
        Consists of a list of arguments and body (both nested Pairs), together
        with a link to the environment in which the procedure was defined.
    """
    def __init__(self, args, body, env):
        self.args = args
        self.body = body
        self.env = env


class BobInterpreter(object):
    """ A Scheme interpreter. After initialization, use the interpret() method
        to interpret parsed Scheme expressions.
    """
    class InterpretError(Exception): pass

    def __init__(self, output_stream=None):
        """ Initialize the interpreter. output_stream is the destination for
            'write' calls in the Scheme code. If None, sys.stdout will be used.
        """
        self.global_env = self._create_global_env()
        
        if output_stream is None:
            import sys
            self.output_stream = sys.stdout
        else:
            self.output_stream = output_stream
    
    def interpret(self, expr):
        """ Interpret the given expression in the current interpreter context 
            and return the result of its evaluation.
        """
        return self._eval(expr, self.global_env)
    
    def _eval(self, expr, env):
        if DEBUG: print('~~~~ Eval called on %s [%s]' % (expr_repr(expr), type(expr)))
        if DEBUG: print('Env:')
        if DEBUG: pprint.pprint(env.binding)

        # Standard Scheme eval (SICP 4.1.1)
        #
        if is_self_evaluating(expr):
            return expr
        elif is_variable(expr):
            return env.lookup_var(expr.value)
        elif is_quoted(expr):
            return text_of_quotation(expr)
        elif is_assignment(expr):
            env.set_var_value(
                var=assignment_variable(expr).value, 
                value=self._eval(assignment_value(expr), env))
            return None
        elif is_definition(expr):
            env.define_var(
                var=definition_variable(expr).value,
                value=self._eval(definition_value(expr), env))
            return None
        elif is_if(expr):
            predicate = self._eval(if_predicate(expr), env)
            if predicate == Boolean(False):
                return self._eval(if_alternative(expr), env)
            else:
                return self._eval(if_consequent(expr), env)
        elif is_cond(expr):
            return self._eval(convert_cond_to_ifs(expr), env)
        elif is_let(expr):
            return self._eval(convert_let_to_application(expr), env)
        elif is_lambda(expr):
            return Procedure(
                        args=lambda_parameters(expr),
                        body=lambda_body(expr),
                        env=env)
        elif is_begin(expr):
            return self._eval_sequence(begin_actions(expr), env)
        elif is_application(expr):
            return self._apply(
                            self._eval(application_operator(expr), env),
                            self._list_of_values(application_operands(expr), env))
        else:
            raise self.InterpretError("Unknown expression in EVAL: %s" % expr)
    
    def _eval_sequence(self, exprs, env):
        # Evaluates a sequence of expressions with _eval and returns the value
        # of the last one
        #
        first_val = self._eval(first_exp(exprs), env)
        if is_last_exp(exprs):
            return first_val
        else:
            return self._eval_sequence(rest_exps(exprs), env)    
    
    def _list_of_values(self, exprs, env):
        # Evaluates a list of expressions with _eval and returns a list of 
        # evaluated results.
        # The order of evaluation is left-to-right
        # 
        if has_no_operands(exprs):
            return None
        else:
            return Pair(self._eval(first_operand(exprs), env),
                        self._list_of_values(rest_operands(exprs), env))

    def _apply(self, proc, args):
        # Standard Scheme apply (SICP 4.1.1)
        #
        if DEBUG: print("~~~~ Applying procedure %s" % proc)
        if DEBUG: print("     with args %s" % expr_repr(args))
        if isinstance(proc, BuiltinProcedure):
            if DEBUG: print("~~~~ Applying builtin procedure %s" % proc.name)
            # The '' builtin gets the current output stream as a custom
            # argument
            #
            return proc.apply(expand_nested_pairs(args))
            
        elif isinstance(proc, Procedure):
            if DEBUG: print("~~~~ Applying procedure with args: %s" % proc.params)
            if DEBUG: print("     and body:\n%s" % expr_repr(proc.body))
            return self._eval_sequence(
                    exprs=proc.body,
                    env=self._extend_env_for_procedure(
                                env=proc.env, 
                                args=proc.args, 
                                args_vals=args))
        else:
            raise self.InterpretError("Unknown procedure type in APPLY: %s" % proc)
    
    def _extend_env_for_procedure(self, env, args, args_vals):
        # Extend an environment with bindings of args -> param_vals. 
        # Creates a new environment linked to the given env.
        # args and param_vals are Scheme lists (nested Pairs)
        #
        new_bindings = {}
        
        while args is not None:
            if args_vals is None:
                raise self.InterpretError('Unassigned parameter in procedure call: %s' % args.first)
            new_bindings[args.first.value] = args_vals.first
            args = args.second
            args_vals = args_vals.second
        
        return Environment(new_bindings, env)

    def _write(self, args):
        # Abides by the builtin procedure calling convention - args is a Python
        # list of arguments.
        #
        self.output_stream.write(expr_repr(args[0]) + '\n')
        return None

    def _create_global_env(self):
        global_binding = {}
        for name, func in builtins_map.items():
            global_binding[name] = BuiltinProcedure(name, func)

        # Add the 'write' builtin which requires access to the VM state 
        #
        global_binding['write'] = BuiltinProcedure('write', self._write)
        return Environment(global_binding)
    

def interpret_code(code_str, output_stream=None):
    """ Convenience function for interpeting a string containing Scheme code.
        Doesn't return anything, so the only visible outcome is side effects
        from the Scheme code (such as invocations of the (write) function).
    """
    parsed_exprs = BobParser().parse(code_str)    
    
    # The interpreter isn't tail-recursive, so ask Python to allow deeper
    # recursive calls
    #
    sys.setrecursionlimit(500000)

    interp = BobInterpreter(output_stream)
    for expr in parsed_exprs:
        interp.interpret(expr)


def interactive_interpreter():
    """ Interactive interpreter 
    """
    interp = BobInterpreter() # by default output_stream is sys.stdout
    parser = BobParser()
    print("Interactive Bob interpreter. Type a Scheme expression or 'quit'")

    while True:
        inp = raw_input("[bob] >> ")
        if inp == 'quit':
            break
        parsed = parser.parse(inp)
        val = interp.interpret(parsed[0])
        if val is None:
            pass
        elif isinstance(val, Procedure):
            print(": <procedure object>")
        else:
            print(":", expr_repr(val))


#-------------------------------------------------------------------------------
if __name__ == '__main__':
    pass

