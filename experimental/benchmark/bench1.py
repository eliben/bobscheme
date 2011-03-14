#-------------------------------------------------------------------------------
# bob: benchmark/bench1.py
#
# Benchmark interpreter vs. compiler/VM
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
import sys, os
from utils import vm_compiler_runner, interpreter_runner, make_barevm_runner

sys.path.insert(0, '../..')
from bob.utils import Timer 
barevm_path = "../barevm/barevm.exe"


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

(write (primecheck 5903))  ; it's a prime!
'''

# Since the interpreter isn't tail-recursive, this is required
sys.setrecursionlimit(200000)

#with Timer('interp'):
    #interpreter_runner(code_str, None)

with Timer('comp_vm'):
    vm_compiler_runner(code_str, None)

with Timer('barevm'):
    make_barevm_runner(barevm_path)(code_str, None)

