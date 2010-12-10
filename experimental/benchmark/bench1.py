#-------------------------------------------------------------------------------
# bob: benchmark/bench1.py
#
# Benchmark interpreter vs. compiler/VM
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys, os
from subprocess import Popen, PIPE
import tempfile
sys.path.insert(0, '../..')

from bob.utils import Timer
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


barevm_path = "../barevm/barevm.exe"
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


code_str = '''
(define (divides k n)
    (= (modulo n k) 0))

(define (primecheck num)
    (define (auxprimecheck divisor)
        (cond 
            ((= divisor num) #t)
            ((divides divisor num) #f)
            (else (auxprimecheck (+ 1 divisor)))))
    (auxprimecheck 2))

(write (primecheck 59053))  ; it's a prime!
'''

# Since the interpreter isn't tail-recursive, this is required
sys.setrecursionlimit(200000)

#with Timer('interp'):
    #interpreter_runner(code_str, None)

with Timer('comp_vm'):
    vm_compiler_runner(code_str, None)

with Timer('barevm'):
    barevm_runner(code_str, None)

