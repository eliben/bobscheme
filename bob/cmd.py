#-------------------------------------------------------------------------------
# bob: cmd.py
#
# Command-line functions
#
# Dustin J. Mitchell (dustin@v.igoro.us)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function

import os, sys
import argparse

from bob import py3compat
from bob.bobparser import BobParser
from bob.bytecode import (Serializer, Deserializer)
from bob.compiler import compile_code
from bob.interpreter import (expr_repr, interpret_code, BobInterpreter,
                             Procedure)
from bob.vm import BobVM


def compile_file(filename=None, out_filename=None, disassemble=False):
    """ Given the name of a .scm file, compile it with the Bob compiler
        to produce a corresponding .bobc file.
    """
    if not filename:
        filename = sys.argv[1]
    code_str = open(filename).read()
    codeobject = compile_code(code_str)
    if disassemble:
        print(codeobject)
        return

    serialized = Serializer().serialize_bytecode(codeobject)

    # Create the output file
    if not out_filename:
        filename_without_ext = os.path.splitext(filename)[0]
        out_filename = filename_without_ext + '.bobc'

    with open(out_filename, 'wb') as out_file:
        out_file.write(serialized)
        print("Output file created: %s" % out_filename)


def interactive_interpreter():
    """ Interactive interpreter 
    """
    interp = BobInterpreter() # by default output_stream is sys.stdout
    parser = BobParser()
    print("Interactive Bob interpreter. Type a Scheme expression or 'quit'")

    while True:
        inp = py3compat.input("[bob] >> ")
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


def interpret_file(filename=None):
    if not filename:
        filename = sys.argv[1]
    with open(filename) as f:
        interpret_code(f.read())


def run_compiled(filename=None):
    """ Given the name of a compiled Bob file (.bobc), run it with the 
        Bob VM with output redirected to stdout.
    """
    if not filename:
        filename = sys.argv[1]
    bytecode = open(filename, 'rb').read()
    codeobject = Deserializer().deserialize_bytecode(bytecode)
    vm = BobVM(output_stream=sys.stdout)
    vm.run(codeobject)


def disassemble_file(filename=None):
    """ Given the name of a compiled Bob file (.bobc), print it in
        human-readable format.
    """
    bytecode = open(filename, 'rb').read()
    codeobject = Deserializer().deserialize_bytecode(bytecode)
    print(codeobject)


DESCRIPTION = '''
Bob is a suite of implementations of the Scheme language in Python'''


def main():
    parser = argparse.ArgumentParser(description=DESCRIPTION)
    parser.add_argument('-c', '--compile',
        help="Compile a scheme file to bytecode", action='store_true')
    parser.add_argument('-d', '--disassemble',
        help="Disassemble a bytecode file", action='store_true')
    parser.add_argument('-o', '--output',
        help="Output filename for copmilation", type=str)
    parser.add_argument('filename', nargs="?",
        help='filename to compile (-c) or run')

    args = parser.parse_args(sys.argv[1:])
    if args.disassemble and args.output:
        parser.error("Cannot specify -o when disassembling")

    if not args.filename:
        if args.compile:
            parser.error("specify a source file to compile")
        interactive_interpreter()
    else:
        is_scm = args.filename.endswith('.scm')
        if args.compile:  # and possibly args.disassemble
            if not is_scm:
                parser.error("can only compile .scm files")
            compile_file(args.filename, args.output, args.disassemble)
        elif args.disassemble:
            if is_scm:
                parser.error("can only disassemble bytecode files")
            disassemble_file(args.filename)
        else:
            if is_scm:
                interpret_file(args.filename)
            else:
                run_compiled(args.filename)


if __name__ == '__main__':
    main()
