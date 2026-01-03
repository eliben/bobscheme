#!/usr/bin/env python3
from bob.bobparser import BobParser
from bob.expr import expr_repr

scmprog = """
(define (factorial n)
    (if (= n 0)
        1
        (* n (factorial (- n 1)))))
(write (factorial 5))
"""

parser = BobParser()
exprs = parser.parse(scmprog)

for expr in exprs:
    print("----")
    print(expr_repr(expr))
