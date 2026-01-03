# -------------------------------------------------------------------------------
# bob: wasmcompiler.py
#
# Compiles Bob expressions (parsed from Scheme text) to WAT (WebAssembly Text).
# It's a drop-in replacement for BobCompiler - it takes a list of Expr objects
# (as produced by BobParser) and produces WAT code (instead of Bob bytecode).
#
# Eli Bendersky (eliben@gmail.com) This code is in the public domain
# -------------------------------------------------------------------------------
from dataclasses import dataclass
from io import StringIO
from typing import List, TextIO

from .expr import Symbol, Pair, Number, Boolean, Nil, expr_repr


class WasmCompiler:
    def __init__(self, stream: TextIO):
        self.stream = stream
        self.indent = 0

        # A list of io.StringIO for the code of each user-defined function. A
        # function at index I is named $user_func_I,
        self.user_funcs = []

        # Compile-time lexical environment, helping us keep track of variable
        # bindings at the point of emission. It's a stack of frames (lists of
        # variable names). At the top of the stack is the innermost scope.
        self.lexical_env = []

        # Tailcall depth counter. When emitting a function call, if this is 0,
        # we can emit a tail call; otherwise, we emit a normal call.
        # This counter is reset for every function; it's incremented when
        # we know that the result of an expression is going to be used in the
        # same frame.
        self.tailcall_pos = 0

    def compile(self, exprlist):
        compiled_exprs = self._comp_exprlist(exprlist)
        return CompiledProcedure(args=[], name="", code=compiled_exprs)

    def _emit_line(self, line: str):
        self.stream.write(" " * self.indent + line + "\n")

    def _emit_text(self, text: str):
        for line in text.splitlines():
            self._emit_line(line)


_builtin_types = r"""
;; PAIR holds the car and cdr of a cons cell.
(type $PAIR (struct (field (mut anyref)) (field (mut anyref))))

;; ENV holds a reference to the parent env, and a list of values.
(type $ENV (struct (field (ref null $ENV)) (field anyref)))

;; CLOSURE holds a reference to the environment, and the function index in
;; the function table.
(type $CLOSURE (struct (field (ref null $ENV)) (field i32)))

;; FUNC is the type of a Scheme function,
(type $FUNC (func (param $arg anyref) (param $env (ref null $ENV)) (result anyref)))

;; TOPLEVEL is the top-level function exported to the host.
(type $TOPLEVEL (func (result i32)))
"""


@dataclass
class BuiltinFunc:
    name: str
    idx: int
    code_templ: str
    code_params: dict[str, str]


_builtins = []


def _register_builtin(name: str, code_templ: str, code_params: dict[str, str]):
    idx = len(_builtins)
    _builtins.append(BuiltinFunc(name, idx, code_templ, code_params))


_car_code = r"""
(func $car (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (struct.get $PAIR 0
        (ref.cast (ref $PAIR)
            (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg)))))
)
"""

_cdr_code = r"""
(func $cdr (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (struct.get $PAIR 1
        (ref.cast (ref $PAIR)
            (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg)))))
)
"""

_cons_code = r"""
(func $cons (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (struct.new $PAIR
        (struct.get $PAIR 0
            (ref.cast (ref $PAIR) (local.get $arg)))
        (struct.get $PAIR 0
            (ref.cast (ref $PAIR)
                (struct.get $PAIR 1 (ref.cast (ref $PAIR) (local.get $arg))))))
)
"""

_nullp_code = r"""
(func $null? (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (ref.is_null (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg))))
    if (result anyref)
        (ref.i31 (i32.const 1))
    else
        (ref.null any)
    end
)
"""

_binop_arith_code = r"""
(func ${NAME} (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (ref.i31 ({BINOP}
        (i31.get_s
            (ref.cast (ref i31)
                (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg)))))
        (i31.get_s
            (ref.cast (ref i31)
                (struct.get $PAIR 0
                    (ref.cast (ref $PAIR)
                        (struct.get $PAIR 1 (ref.cast (ref $PAIR) (local.get $arg)))))))))
)
"""

_binop_cmp_code = r"""
(func ${NAME} (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    ({BINOP}
        (i31.get_s
            (ref.cast (ref i31)
                (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg)))))
        (i31.get_s
            (ref.cast (ref i31)
                (struct.get $PAIR 0
                    (ref.cast (ref $PAIR)
                        (struct.get $PAIR 1 (ref.cast (ref $PAIR) (local.get $arg))))))))
    if (result anyref)
        (ref.i31 (i32.const 1))
    else
        (ref.null any)
    end
)
"""

_register_builtin("car", _car_code, {})
_register_builtin("cdr", _cdr_code, {})
_register_builtin("cons", _cons_code, {})
_register_builtin("null?", _nullp_code, {})
_register_builtin(
    "+",
    _binop_arith_code,
    {"NAME": "_add", "BINOP": "i32.add"},
)
_register_builtin(
    "-",
    _binop_arith_code,
    {"NAME": "_sub", "BINOP": "i32.sub"},
)
_register_builtin(
    "*",
    _binop_arith_code,
    {"NAME": "_mul", "BINOP": "i32.mul"},
)
_register_builtin(
    "<",
    _binop_cmp_code,
    {"NAME": "_lt", "BINOP": "i32.lt_s"},
)
_register_builtin(
    ">",
    _binop_cmp_code,
    {"NAME": "_gt", "BINOP": "i32.gt_s"},
)
_register_builtin(
    "=",
    _binop_cmp_code,
    {"NAME": "_eq", "BINOP": "i32.eq"},
)
