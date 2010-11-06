#-------------------------------------------------------------------------------
# bob: bytecode.py
#
# Bytecode objects and utilities for the Bob VM.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
from .utils import pack_word, unpack_word, byte_literal, get_bytes_from_iterator
from .expr import (Pair, Boolean, Symbol, Number, expr_repr)


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
        self.name = ''
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


# The serialization scheme is similar to Python's marshalling of code. Each
# object is serialized by prepending a single "type" byte, followed by 
# the object's serialized representation. See the code of Serializer for the
# details of how each type is serialized - it's pretty simple!
#
# A serialized bytecode consists of a string containing a magic constant
# followed by the serialized top-level CodeObject in the bytecode. This is 
# created by Serializer.serialize_bytecode
# 
# The Deserializer class can convert a serialized string back into a 
# CodeObject, in Deserializer.deserialize_bytecode
#
# The "magic" constant starting any serialized Bob bytecode consists of a 
# version in the high two bytes and 0B0B in the low two bytes.
#
# IMPORTANT: For compatibility with Python 2.6 and 3.x, bytecode is serialized
# into 'bytes' objects.
#
MAGIC_CONST = 0x00010B0B

TYPE_NULL       = b'0'
TYPE_BOOLEAN    = b'b'
TYPE_STRING     = b's'
TYPE_SYMBOL     = b'S'
TYPE_NUMBER     = b'n'
TYPE_PAIR       = b'p'
TYPE_INSTR      = b'i'
TYPE_SEQUENCE   = b'['
TYPE_CODEOBJECT = b'c'


class Serializer(object):
    """ Serializes a CodeObject to a string.
    """
    # Each function beginning with _s serializes some type and returns 
    # a string representing the serialized object.
    #
    def __init__(self):
        # Allows dispatching serialization of Bob objects according to their
        # types
        #
        self._serialize_type_dispatch = {
            type(None):     self._s_null,
            Boolean:        self._s_boolean,
            Number:         self._s_number,
            Symbol:         self._s_symbol,
            Pair:           self._s_pair,
            Instruction:    self._s_instruction,
            CodeObject:     self._s_codeobject,
            type([]):       self._s_sequence,
            type(''):       self._s_string,
        }

    def serialize_bytecode(self, codeobject):
        """ Serialize a top-level CodeObject into a string that can be written
            into a file.
        """
        s = self._s_word(MAGIC_CONST)
        s += self._s_codeobject(codeobject)
        return s
    
    def _s_word(self, wordvalue):
        """ word - a 32-bit integer, serialized in 4 bytes as little-endian
        """
        return pack_word(wordvalue, big_endian=False)

    def _s_string(self, string):
        """ string - a Python string, used for representing names in code
            objects and for Bob Symbol objects.
        """
        return TYPE_STRING + self._s_word(len(string)) + string.encode('ascii')
           
    def _s_object(self, obj):
        """ Generic dispatcher for serializing an arbitrary object
        """
        return self._serialize_type_dispatch[type(obj)](obj)

    def _s_null(self, *args):
        return TYPE_NULL

    def _s_boolean(self, bool):
        return TYPE_BOOLEAN + (b'\x01' if bool.value else b'\x00')

    def _s_number(self, number):
        return TYPE_NUMBER + self._s_word(number.value)

    def _s_symbol(self, symbol):
        return TYPE_SYMBOL + self._s_word(len(symbol.value)) + symbol.value.encode('ascii')

    def _s_pair(self, pair):
        return (    TYPE_PAIR + 
                    self._s_object(pair.first) + 
                    self._s_object(pair.second))

    def _s_sequence(self, seq):
        """ A sequence is just a Python list, used for serializing parts
            of code objects.
        """
        s_seq = b''.join(self._s_object(obj) for obj in seq)
        return TYPE_SEQUENCE + self._s_word(len(seq)) + s_seq

    def _s_instruction(self, instr):
        """ Instructions are mapped into words, with the opcode taking 
            the high byte and the argument the low 3 bytes.
        """
        arg = instr.arg or 0
        instr_word = (instr.opcode << 24) | (arg & 0xFFFFFF)
        return TYPE_INSTR + self._s_word(instr_word)

    def _s_codeobject(self, codeobject):
        s = TYPE_CODEOBJECT
        s += self._s_string(codeobject.name)
        s += self._s_sequence(codeobject.args)
        s += self._s_sequence(codeobject.constants)
        s += self._s_sequence(codeobject.varnames)
        s += self._s_sequence(codeobject.code)
        return s


class Deserializer(object):
    """ Deserializes a CodeObjct from a string
    """
    
    class DeserializationError(Exception): pass

    # Each function beginning with _d deserializes an object from the given
    # stream. The stream is represented as an iterator, from which any
    # deserialization function takes exactly as much as it needs.
    # These functions are being called *after* the type specifier has been
    # stripped from the stream.
    #
    def __init__(self):
        self._deserialize_type_dispatch = {
            TYPE_NULL:          self._d_null,
            TYPE_BOOLEAN:       self._d_boolean, 
            TYPE_NUMBER:        self._d_number,
            TYPE_SYMBOL:        self._d_symbol,
            TYPE_PAIR:          self._d_pair,
            TYPE_INSTR:         self._d_instruction,
            TYPE_CODEOBJECT:    self._d_codeobject,
            TYPE_SEQUENCE:      self._d_sequence,
            TYPE_STRING:        self._d_string,
        }
        pass

    def deserialize_bytecode(self, str):
        """ Given a string with a serialized code object, converts it onto
            a CodeObject.
        """
        try:
            stream = iter(str)
            magic = self._d_word(stream)

            if magic != MAGIC_CONST:
                raise self.DeserializationError("Magic constant does't match")

            self._match_type(stream, TYPE_CODEOBJECT)
            return self._d_codeobject(stream)
        except StopIteration as e:
            raise self.DeserializationError("Unexpected end of serialized stream")
        
    def _match_type(self, stream, type):
        b = next(stream)
        if byte_literal(b) != type:
            raise self.DeserializationError("Expected type '%s' not matched" % type)

    def _d_word(self, stream):
        bytes = get_bytes_from_iterator(stream, 4)
        return unpack_word(bytes, big_endian=False)

    def _d_object(self, stream):
        """ Generic dispatcher for deserializing an arbitrary object.
        """
        type = get_bytes_from_iterator(stream, 1)
        return self._deserialize_type_dispatch[type](stream)

    def _d_null(self, stream):
        # NULL has nothing in the stream after the type
        return None

    def _d_boolean(self, stream):
        return Boolean(get_bytes_from_iterator(stream, 1) == b'\x01')

    def _d_string(self, stream):
        len = self._d_word(stream)
        return get_bytes_from_iterator(stream, len).decode('ascii')

    def _d_number(self, stream):
        return Number(self._d_word(stream))

    def _d_symbol(self, stream):
        return Symbol(self._d_string(stream))

    def _d_pair(self, stream):
        first = self._d_object(stream)
        second = self._d_object(stream)
        return Pair(first, second)

    def _d_sequence(self, stream):
        len = self._d_word(stream)
        return [self._d_object(stream) for i in range(len)]

    def _d_instruction(self, stream):
        word = self._d_word(stream)
        return Instruction(word >> 24, word & 0xFFFFFF)
    
    def _d_codeobject(self, stream):
        co = CodeObject()
        self._match_type(stream, TYPE_STRING)
        co.name = self._d_string(stream)

        seqs = []
        for i in range(4):
            self._match_type(stream, TYPE_SEQUENCE)
            seqs.append(self._d_sequence(stream))
        co.args, co.constants, co.varnames, co.code = seqs

        return co


#-----------------------------------------------------------------------------
if __name__ == '__main__':
    ss = Serializer()
    print(ss._s_boolean(Boolean(False)).encode('hex'))
    print(ss._s_instruction(Instruction(OP_CALL, 34)).encode('hex'))


