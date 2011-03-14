#-------------------------------------------------------------------------------
# bob: benchmark/utils.py
#
# Common utilities for benchmarking in Bob
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys, os
from subprocess import Popen, PIPE
import tempfile
sys.path.insert(0, '../..')
from bob.bytecode import Serializer
from bob.interpreter import interpret_code
from bob.compiler import compile_code
from bob.vm import BobVM

def vm_compiler_runner(code, ostream):
    codeobject = compile_code(code)
    vm = BobVM(output_stream=ostream)
    vm.run(codeobject)


def interpreter_runner(code, ostream):
    interpret_code(code, output_stream=ostream)


def make_barevm_runner(barevm_path):
    def barevm_runner(code, ostream):
        codeobject = compile_code(code)
        serialized = Serializer().serialize_bytecode(codeobject)

        # Get a temporary filename and write the serialized codeobject
        # into it
        #
        fileobj, filename = tempfile.mkstemp()
        os.write(fileobj, serialized)
        os.close(fileobj)

        vm_proc = Popen([barevm_path, filename], stdout=PIPE)
        vm_output = vm_proc.stdout.read()

        if ostream:
            ostream.write(vm_output)
        os.remove(filename)
    return barevm_runner
