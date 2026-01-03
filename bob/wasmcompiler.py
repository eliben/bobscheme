# -------------------------------------------------------------------------------
# bob: wasmcompiler.py
#
# Compiles Bob expressions (parsed from Scheme text) to WAT (WebAssembly Text).
# It's a drop-in replacement for BobCompiler - it takes a list of Expr objects
# (as produced by BobParser) and produces WAT code as a string (instead of
# Bob bytecode).
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
# -------------------------------------------------------------------------------

from .expr import Symbol, Pair, Number, Boolean, Nil, expr_repr


class WasmCompiler:
    pass


if __name__ == "__main__":
    scmprog = """
(define (factorial n)
    (if (= n 0)
        1
        (* n (factorial (- n 1)))))
(write (factorial 5))
"""
    from .bobparser import BobParser

    parser = BobParser()
    exprs = parser.parse(scmprog)

    for expr in exprs:
        print("----")
        print(expr_repr(expr))
