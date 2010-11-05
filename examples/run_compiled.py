#!/usr/bin/python
import os, sys

sys.path.insert(0, '..')
from bob.vm import BobVM
from bob.bytecode import Deserializer


def run_compiled(filename):
    """ Given the name of a compiled Bob file (.bobc), run it with the 
        Bob VM with output redirected to stdout.
    """
    bytecode = open(filename, 'rb').read()
    codeobject = Deserializer().deserialize_bytecode(bytecode)
    vm = BobVM(output_stream=sys.stdout)
    vm.run(codeobject)


if __name__ == '__main__':
    run_compiled(sys.argv[1])

    
