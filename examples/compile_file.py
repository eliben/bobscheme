#!/usr/bin/python
import os, sys

sys.path.insert(0, '..')
from bob.compiler import compile_code
from bob.bytecode import Serializer


def compile_file(filename):
    """ Given the name of a .scm file, compile it with the Bob compiler
        to produce a corresponding .bobc file.
    """
    code_str = open(filename).read()
    codeobject = compile_code(code_str)
    serialized = Serializer().serialize_bytecode(codeobject)

    # Create the output file
    filename_without_ext = os.path.splitext(filename)[0]
    out_filename = filename_without_ext + '.bobc'

    with open(out_filename, 'wb') as out_file:
        out_file.write(serialized)


if __name__ == '__main__':
    compile_file(sys.argv[1])


    
