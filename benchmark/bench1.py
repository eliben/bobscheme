#-------------------------------------------------------------------------------
# bob: benchmark/bench1.py
#
# Benchmark interpreter vs. compiler/VM
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys

sys.path.insert(0, '..')
from bob.utils import Timer
from bob.interpreter import interpret_code
from bob.compiler import compile_code
from bob.vm import BobVM


def vm_compiler_runner(code, ostream):
    codeobject = compile_code(code)
    vm = BobVM(output_stream=ostream)
    vm.run(codeobject)

def interpreter_runner(code, ostream):
    interpret_code(code, output_stream=ostream)


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

(write (primecheck 2711))  ; it's a prime!
'''

# Since the interpreter isn't tail-recursive, this is required
sys.setrecursionlimit(200000)

with Timer('interp'):
    interpreter_runner(code_str, None)

with Timer('comp_vm'):
    vm_compiler_runner(code_str, None)


