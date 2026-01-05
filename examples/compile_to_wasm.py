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

from bob.wasmcompiler import WasmCompiler

# Create a stringio to hold the wasm output
import io

output_stream = io.StringIO()

compiler = WasmCompiler(output_stream)
wasm_tpl = compiler.compile(exprs)
print(wasm_tpl)
print(expr_repr(wasm_tpl))
