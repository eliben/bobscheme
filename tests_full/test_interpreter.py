#!/usr/bin/env python
#-------------------------------------------------------------------------------
# bob: tests_full/test_interpreter.py
#
# Run the full tests for the interpreter module of Bob
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys
from testcases_utils import run_all_tests

sys.path.insert(0, '..')
from bob.interpreter import interpret_code


def interpreter_runner(code, ostream):
    interpret_code(code, output_stream=ostream)


run_all_tests(runner=interpreter_runner)
