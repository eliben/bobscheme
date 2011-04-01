import sys, os
from utils import vm_compiler_runner, interpreter_runner, make_barevm_runner

sys.path.insert(0, '../..')
from bob.utils import Timer 
barevm_path = "../barevm/barevm.exe"

code_str = '''
(define (foobar)
    (+ 2 (+ 5 6)))

(define (barbaz i)
    (__debug-vm))

(write (write 2))
(define a 12)
(define j (cons 1 (cons 2 (cons 3 5))))
(define b (+ a 5))
(__debug-gc #t)
(foobar)
(write 'running-gc)
(__run-gc)
(__debug-gc #t)
'''

make_barevm_runner(barevm_path)(code_str, sys.stdout)

