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
        """ Evaluate a scheme expression in an environment
            and return the result.
            If the return result is just another scheme expression
            to evaluate (e.g. a tail call) then loop and evaluate
            the new scheme expression
        """
        while True:
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
                    # replace current expression with
                    #  the if alternative and loop
                    expr = if_alternative(expr)
                else:
                    # replace current expression with
                    #  the if consequent and loop
                    expr = if_consequent(expr)
            elif is_cond(expr):
                # replace current expression with
                #  the converted cond and loop
                expr = convert_cond_to_ifs(expr)
            elif is_let(expr):
                # replace current expression with
                #  the converted let and loop
                expr = convert_let_to_application(expr)
            elif is_lambda(expr):
                return Procedure(
                        args=lambda_parameters(expr),
                        body=lambda_body(expr),
                        env=env)
            elif is_begin(expr):
                # _eval_sequence evaluates in order a sequence of expressions with _eval
                #  but returns the last expression unevaluated
                # loop and evaluate the last expression
                expr = self._eval_sequence(begin_actions(expr), env)
            elif is_application(expr):
                # if the expression operator is a builtin,
                #  _apply returns (True, builtin_return_value, None)
                # otherwise, it returns
                #  (False, lambda_body_last_expression, lambda_environment)
                (is_builtin, expr, env) = self._apply(
                            self._eval(application_operator(expr), env),
                            self._list_of_values(application_operands(expr), env))
                if is_builtin:
                    return expr
                # if is_builtin == False then loop and evaluate
                #  lambda_body_last_expression in lambda_environment
            else:
                raise self.InterpretError("Unknown expression in EVAL: %s" % expr)

    def _eval_sequence(self, exprs, env):
        """ Evaluates all but the last of a sequence of expressions with _eval
            Returns the last expression for _eval to evaluate as a tail call
        """
        while True:
            if is_last_exp(exprs):
                return first_exp(exprs)
            else:
                self._eval(first_exp(exprs), env)
                exprs = rest_exps(exprs)

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
            # Return a tuple to _eval with the result
            #  and is_builtin == True
            return (True, proc.apply(expand_nested_pairs(args)), None)

        elif isinstance(proc, Procedure):
            if DEBUG: print("~~~~ Applying procedure with args: %s" % proc.params)
            if DEBUG: print("     and body:\n%s" % expr_repr(proc.body))
            # assemble the extented environment
            new_env = self._extend_env_for_procedure(
                                env=proc.env,
                                args=proc.args,
                                args_vals=args)
            # Use _eval_sequence to evaluate pre-final expressions
            #  in the lambda body and get the final expression
            # Return a tuple to _eval with is_builtin == False,
            #  the final lambda body expression, and the lambda environment
            return (False,
                    self._eval_sequence(exprs=proc.body, env=new_env),
                    new_env)
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
