#!/usr/bin/env python
# -------------------------------------------------------------------------------
# bob: tests_full/test_vm_compiler.py
#
# Run the full tests for the compiler and VM of Bob
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
# -------------------------------------------------------------------------------
import sys
from testcases_utils import run_tests

from bob.compiler import compile_code
from bob.vm import BobVM
from bob.bytecode import Serializer, Deserializer


def vm_compiler_runner(code, ostream):
    codeobject = compile_code(code)

    # Run the code through (de+)serialization to exercise it.
    ser = Serializer().serialize_bytecode(codeobject)
    codeobject = Deserializer().deserialize_bytecode(ser)
    vm = BobVM(output_stream=ostream)
    vm.run(codeobject)


run_tests(runner=vm_compiler_runner)
