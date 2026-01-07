#!/usr/bin/env python3
from bob.bobparser import BobParser
from bob.expr import expr_repr

scmprog = """
(write 5)
"""

parser = BobParser()
exprs = parser.parse(scmprog)

for expr in exprs:
    print("----")
    print(expr_repr(expr))

from bob.wasmcompiler import WasmCompiler, expr_tree_repr

# Create a stringio to hold the wasm output
import io

output_stream = io.StringIO()

compiler = WasmCompiler(output_stream)
compiler.compile(exprs)

print("WASM output:")
print(output_stream.getvalue())
