#-------------------------------------------------------------------------------
# bob: bytecode.py
#
# Bytecode objects and utilities for the Bob VM.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from expr import expr_repr


OP_CONST    = 0x00
OP_LOADVAR  = 0x10
OP_STOREVAR = 0x11
OP_DEFVAR   = 0x12
OP_FUNCTION = 0x20
OP_POP      = 0x30
OP_JUMP     = 0x40
OP_FJUMP    = 0x41
OP_RETURN   = 0x50
OP_CALL     = 0x51


_opcode2str_map = {
    OP_CONST:       'CONST',
    OP_LOADVAR:     'LOADVAR',
    OP_STOREVAR:    'STOREVAR',
    OP_DEFVAR:      'DEFVAR',
    OP_FUNCTION:    'FUNCTION',
    OP_POP:         'POP',
    OP_JUMP:        'JUMP',
    OP_FJUMP:       'FJUMP',
    OP_RETURN:      'RETURN',
    OP_CALL:        'CALL',
}


def opcode2str(opcode):
    return _opcode2str_map[opcode]


class Instruction(object):
    """ A bytecode instruction. The opcode is one of the OP_... constants 
        defined above. 
        
        The argument will be arbitrary when used in a CompiledProcedure, but
        is always numeric when the Instruction is part of a CodeObject.
    """
    def __init__(self, opcode, arg):
        self.opcode = opcode
        self.arg = arg


class CodeObject(object):
    """ A code object is a Scheme procedure in its compiled and assembled form,
        suitable by execution by the VM.
      
        name: 
            Procedure name for debugging. Some procedures are anonymous (were
            defined by a 'lambda' and not 'define' and don't have a name).
            The top-level procedure that represents a whole file also doesn't
            have a name.
        
        args:
            A list of argument names (strings) for the procedure.
        
        code:
            A list of Instruction objects.
        
        constants:
            A list of constants. Constants are either Scheme expressions 
            (as defined by the types in the expr module) or CodeObjects (for
            compiled procedures). The instructions in the code reference 
            constants by their index in this list.
        
        varnames:
            A list of strings specifying variable names referenced in the code.
            The variables are referenced by their index in this list.
    """
    def __init__(self):
        self.name = None
        self.args = []
        self.code = []
        self.constants = []
        self.varnames = []

    def __repr__(self, nesting=0):
        repr = ''
        prefix = ' ' * nesting
        
        repr += prefix + '----------\n'
        repr += prefix + 'CodeObject: ' + ('' if self.name is None else self.name) + '\n'
        repr += prefix + 'Args: ' + str(self.args) + '\n'

        for offset, instr in enumerate(self.code):
            repr += prefix + '  %4s %-12s ' % (offset, opcode2str(instr.opcode))
            
            if instr.opcode == OP_CONST:
                repr += '%4s {= %s}\n' % (instr.arg, expr_repr(self.constants[instr.arg]))
            elif instr.opcode in (OP_LOADVAR, OP_STOREVAR, OP_DEFVAR):
                repr += '%4s {= %s}\n' % (instr.arg, self.varnames[instr.arg])
            elif instr.opcode in (OP_FJUMP, OP_JUMP):
                repr += '%4s\n' % instr.arg
            elif instr.opcode == OP_CALL:
                repr += '%4s\n' % instr.arg
            elif instr.opcode in (OP_POP, OP_RETURN):
                repr += '\n'
            elif instr.opcode == OP_FUNCTION:
                # Recursively print out another code object
                repr += '%4s {=\n' % instr.arg
                repr += self.constants[instr.arg].__repr__(nesting + 8)
            else:
                assert False, "Unexpected opcode %s" % instr.opcode
        
        repr += prefix + '----------\n'
        return repr





