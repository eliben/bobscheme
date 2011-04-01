#!/usr/bin/env python
#-------------------------------------------------------------------------------
# bob: tests_full/test_barevm.py
#
# Run the full tests for the Bob bare VM
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import os, sys
from subprocess import Popen, PIPE
import tempfile
from testcases_utils import run_all_tests

sys.path.insert(0, '..')
from bob.compiler import compile_code
from bob.bytecode import Serializer


def make_runner(barevm_path):
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

        ostream.write(vm_output)
        os.remove(filename)
    return barevm_runner


if __name__ == '__main__':
    barevm_path = "../barevm/barevm"
    barevm_runner = make_runner(barevm_path)

    run_all_tests(barevm_runner)

