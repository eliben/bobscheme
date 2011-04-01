#!/usr/bin/env python
# The following import is for compatibility with both Python 2.6 and 3.x
from __future__ import print_function
import os, sys

sys.path.insert(0, '..')
from bob.interpreter import interpret_code


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Expecting a scheme file to interpret")
    else:
        with open(sys.argv[1]) as f:
            interpret_code(f.read())

