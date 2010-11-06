#-------------------------------------------------------------------------------
# bob: compiler.py
#
# Scheme compiler. 
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
from .utils import flatten, list_find_or_append
from .expr import *
from .bytecode import *
from .bobparser import BobParser


DEBUG = False


class CompiledLabel(object):
    def __init__(self, name):
        self.name = name
    
    def __repr__(self):
        return str(self.name)


class CompiledProcedure(object):
    """ Represents a compiled procedure.
    
        A procedure contains arguments (list of symbol names), code (list
        of compiled instructions) and optionally a name (for debugging).
    """
    def __init__(self, args, code, name=''):
        self.args = args
        self.code = code
        self.name = name

    def __repr__(self, nesting=0):
        """ Obtain a string representation of a compiled procedure. 
        """
        repr = ''
        prefix = ' ' * nesting
        
        repr += prefix + '----------\n'
        repr += prefix + 'Proc: ' + ('' if self.name is None else self.name) + '\n'
        repr += prefix + 'Args: ' + str(self.args) + '\n'
        
        for instr in self.code:
            if isinstance(instr, CompiledLabel):
                repr += prefix + '  %s:\n' % instr.name
            else:
                assert isinstance(instr, Instruction)
                repr += prefix + '    ' + '%-12s' % opcode2str(instr.opcode)
                if instr.arg is None:
                    repr += '\n'
                elif instr.opcode == OP_FUNCTION:
                    repr += '\n'
                    # Note: calling __repr__ directly in order to pass it the
                    # nesting (the repr() built-in doesn't pass on additional
                    # arguments)
                    #
                    repr += instr.arg.__repr__(nesting + 8)
                else:
                    if is_scheme_expr(instr.arg):
                        arg_repr = expr_repr(instr.arg)
                    else:
                        arg_repr = str(instr.arg)
                    repr += arg_repr + '\n'
        
        repr += prefix + '----------\n'
        return repr


class BobCompiler(object):
    """ A Scheme compiler. 
    """
    class CompileError(Exception): pass

    def __init__(self):
        self.labelstate = 0
        
    def compile(self, exprlist):
        """ Compile a list of parsed expressions (what's returned by 
            BobParser.parse) into a single argument-less CompiledProcedure 
            with the instructions stored in its .code attribute.
        """
        compiled_exprs = self._comp_exprlist(exprlist)
        return CompiledProcedure(args=[], name='', code=compiled_exprs)

    def _make_label(self, prefix=None):
        prefix = prefix or 'LABEL'
        self.labelstate += 1
        return CompiledLabel('%s%s' % (prefix, self.labelstate))

    def _instr(self, opcode, arg=None):
        """ A helper creator for instructions.
        """
        return [Instruction(opcode, arg)]
        
    def _instr_seq(self, *args):
        """ Create a sequence (list) of instructions, where each argument can
            be either a single instruction or a sequence of instructions.
        """
        return list(flatten(args))

    def _comp(self, expr):
        """ Compile an expression.
        
            Always returns a (Python) list of instructions.
        """
        if DEBUG: print('~~~~ Comp called on %s [%s]' % (expr_repr(expr), type(expr)))

        if is_self_evaluating(expr):
            return self._instr(OP_CONST, expr)
        elif is_variable(expr):
            return self._instr(OP_LOADVAR, expr)
        elif is_quoted(expr):
            return self._instr(OP_CONST, text_of_quotation(expr))
        elif is_assignment(expr):
            return self._instr_seq(
                            self._comp(assignment_value(expr)),
                            self._instr(OP_STOREVAR, assignment_variable(expr)))
        elif is_definition(expr):
            return self._comp_definition(expr)
        elif is_if(expr):
            return self._comp_if(expr)
        elif is_cond(expr):
            return self._comp(convert_cond_to_ifs(expr))
        elif is_let(expr):
            return self._comp(convert_let_to_application(expr))
        elif is_lambda(expr):
            return self._comp_lambda(expr)
        elif is_begin(expr):
            return self._comp_begin(begin_actions(expr))
        elif is_application(expr):
            return self._comp_application(expr)
        else:
            raise self.CompileError("Unknown expression in COMPILE: %s" % expr)
    
    def _comp_lambda(self, expr):
        # The lambda parameters are in Scheme's nested Pair format. Convert
        # them into a normal Python list
        #
        args = expand_nested_pairs(lambda_parameters(expr))
        arglist = []
        
        # Some sanity checking: only symbol arguments are supported
        #
        for sym in args:
            if isinstance(sym, Symbol):
                arglist.append(sym.value)
            else:
                raise self.CompileError("Expected symbol in argument list, got: %s" % expr_repr(sym))
        
        # For the code - compile lambda body as a sequence and append a RETURN
        # instruction to the end
        #
        proc_code = self._instr_seq(self._comp_begin(lambda_body(expr)),
                                    self._instr(OP_RETURN))
        
        return self._instr( OP_FUNCTION, 
                            CompiledProcedure(
                                args=arglist,
                                code=proc_code))

    def _comp_begin(self, exprs):
        # To compile a 'begin' we append the compiled versions of all the 
        # expressions in it, with a POP instruction inserted after each one 
        # except the last. 
        #
        exprlist = expand_nested_pairs(exprs, recursive=False)
        return self._comp_exprlist(exprlist)

    def _comp_exprlist(self, exprlist):
        instr_pop_pairs = list(self._instr_seq(self._comp(expr), self._instr(OP_POP)) for expr in exprlist)
        instrs = self._instr_seq(*instr_pop_pairs)        
        return instrs[:-1] if len(instrs) > 0 else instrs

    def _comp_definition(self, expr):        
        compiled_val = self._comp(definition_value(expr))
        
        # If the value is a procedure (a lambda), assign its .name attribute
        # to the variable name (for debugging)
        #
        var = definition_variable(expr)
        if (    isinstance(compiled_val[-1], Instruction) and 
                isinstance(compiled_val[-1].arg, CompiledProcedure)):
            compiled_val[-1].arg.name = var.value
        
        return self._instr_seq(compiled_val, self._instr(OP_DEFVAR, var))

    def _comp_if(self, expr):
        label_else = self._make_label()
        label_after_else = self._make_label()
        
        return self._instr_seq(
                        self._comp(if_predicate(expr)),
                        self._instr(OP_FJUMP, label_else),
                        self._comp(if_consequent(expr)),
                        self._instr(OP_JUMP, label_after_else),
                        [label_else],
                        self._comp(if_alternative(expr)),
                        [label_after_else])
    
    def _comp_application(self, expr):
        args = expand_nested_pairs(application_operands(expr), recursive=False)
        compiled_args = self._instr_seq(*[self._comp(arg) for arg in args])
        compiled_op = self._comp(application_operator(expr))
        return self._instr_seq(
                        compiled_args,
                        compiled_op,
                        self._instr(OP_CALL, len(args)))


class BobAssembler(object):
    def assemble(self, proc):
        """ Assemble a compiled procedure into a CodeObject
        """
        label_offsets = self._compute_label_offsets(proc)
        return self._assemble_to_code(proc, label_offsets)
        
    def _compute_label_offsets(self, proc):
        """ The first pass of the assembler - compute addresses (offsets) of all 
            labels in the code. Return a dict mapping label name -> offset.
        """
        d = {}
        offset = 0
        for instr in proc.code:
            if isinstance(instr, CompiledLabel):
                d[instr.name] = offset
            else:
                offset += 1
        return d

    def _assemble_to_code(self, proc, label_offsets):
        """ The second and main pass of the assembler - translate the compiled
            procedure int a CodeObject.
        """
        co = CodeObject()
        co.name = proc.name
        co.args = proc.args
        
        for instr in proc.code:
            if isinstance(instr, CompiledLabel):
                continue
            if instr.opcode == OP_CONST:
                # Don't try to save space by folding similar pairs together
                # since this is against the semantics of Scheme: two different
                # Pairs, even with the same elements, must be distinct for eqv?
                #
                if isinstance(instr.arg, Pair):
                    co.constants.append(instr.arg)
                    arg = len(co.constants) - 1
                else:
                    arg = list_find_or_append(co.constants, instr.arg)
            elif instr.opcode in (OP_LOADVAR, OP_STOREVAR, OP_DEFVAR):
                arg = list_find_or_append(co.varnames, instr.arg.value)
            elif instr.opcode == OP_FUNCTION:
                # Recursively assemble the CompiledProcedure referred to by
                # this instruction and shove it into the constants list.
                #
                co.constants.append(self.assemble(instr.arg))
                arg = len(co.constants) - 1
            elif instr.opcode in (OP_FJUMP, OP_JUMP):
                arg = label_offsets[instr.arg.name]
            elif instr.opcode == OP_CALL:
                arg = instr.arg
            elif instr.opcode in (OP_POP, OP_RETURN):
                arg = None
            else:
                assert False, "Unexpected opcode %s" % instr.opcode
                
            co.code.append(Instruction(instr.opcode, arg))

        return co


def compile_code(code_str):
    """ Convenience function for compiling (& assembling) a string containing 
        Scheme code into a code object.
    """
    parsed_exprs = BobParser().parse(code_str)    
    compiled = BobCompiler().compile(parsed_exprs)
    
    return BobAssembler().assemble(compiled)


#-------------------------------------------------------------------------------
if __name__ == '__main__':
    pass
