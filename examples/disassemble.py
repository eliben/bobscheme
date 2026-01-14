#!/usr/bin/env python3
import sys
from bob.cmd import disassemble_file

if __name__ == "__main__":
    # Disassemble the given .bobc file
    disassemble_file(sys.argv[1])
