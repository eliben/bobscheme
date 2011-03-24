#!/usr/bin/env python
#-------------------------------------------------------------------------------
# bob: tests_full/test_vm_compiler.py
#
# Run the full tests for the compiler and VM of Bob
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys
from testcases_utils import run_all_tests

sys.path.insert(0, '..')
from bob.compiler import compile_code
from bob.vm import BobVM
from bob.bytecode import Serializer, Deserializer


def vm_compiler_runner(code, ostream):
    codeobject = compile_code(code)

    # Run the code through (de+)serialization to test how that works 
    # too
    #
    ser = Serializer().serialize_bytecode(codeobject)
    codeobject = Deserializer().deserialize_bytecode(ser)
    vm = BobVM(output_stream=ostream)
    vm.run(codeobject)


run_all_tests(runner=vm_compiler_runner)

