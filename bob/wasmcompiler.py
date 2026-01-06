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

from .expr import *


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
        # print("tree0\n", expr_tree_repr(exprlist[0]))
        # print("tree1\n", expr_tree_repr(exprlist[1]))
        # print(exprlist)
        nestedlist = make_nested_pairs(*exprlist)
        print("tree_nested\n", expr_tree_repr(nestedlist))
        tpl = self._expand_block(nestedlist)
        return tpl

    def _expand_block(self, exprlist):
        # Sequence of expressions. We iterate over it from the last to the first
        # to build a nested BEGIN structure (in 'result'). For each expression,
        # if it's a definition, _expand_definition expands it and returns the
        # defined names, which we prepend to the names list.
        # print("expanding exprlist:\n", expr_tree_repr(exprlist))
        if not isinstance(exprlist, Pair):
            return self._expand_expr(exprlist)

        result = None
        names = None
        for expr in reverse_iter_pairs(exprlist):
            # print("expanding expr:\n", expr_tree_repr(expr))
            expr, def_name = self._expand_definition(expr)
            if result is None:
                result = expr
            else:
                result = Pair(Symbol("begin"), Pair(expr, result))
            if def_name is not None:
                names = Pair(def_name, names)

        # No internal definitions? Just return the result.
        if names is None:
            return result

        # There are internal definitions, and the names they define are in
        # 'names'. We need to wrap the result in a lambda that takes these
        # names as parameters, and then call this lambda with empty lists
        # as values (_expand_definition emitted set! for each definition that
        # executes at runtime).

        # Build the argument list for the lambda. It's a list of '() constants
        # of the same length as 'names'.
        args = None
        for name in iter_pairs(names):
            args = Pair(Symbol("()"), args)

        # Return the application (lamba args).
        return Pair(make_lambda(names, result), args)

    def _expand_definition(self, expr):
        if is_definition(expr):
            def_name = definition_variable(expr)
            def_value = definition_value(expr)

            setbang = make_assignment(def_name, self._expand_expr(def_value))
            return setbang, def_name

        # Not a definition, forward to _expand_expr.
        return self._expand_expr(expr), None

    def _expand_expr(self, expr):
        if isinstance(expr, Symbol):
            return expr
        elif is_self_evaluating(expr):
            return expr

        assert isinstance(expr, Pair)
        if isinstance(expr.first, Symbol):
            match expr.first.value:
                case "lambda":
                    return make_lambda(
                        lambda_parameters(expr),
                        self._expand_block(lambda_body(expr)),
                    )
                case "if":
                    return make_if(
                        self._expand_expr(if_predicate(expr)),
                        self._expand_expr(if_consequent(expr)),
                        self._expand_expr(if_alternative(expr)),
                    )
                case "begin":
                    return self._expand_block(begin_actions(expr))
                case "set!":
                    return make_assignment(
                        assignment_variable(expr),
                        self._expand_expr(assignment_value(expr)),
                    )

        return self._expand_list(expr)

    def _expand_list(self, sexp):
        # Recurisve list expansion: expand head with _expand_expr, and tail
        # with _expand_list.
        if sexp is None:
            return None
        assert isinstance(sexp, Pair)
        return Pair(
            self._expand_expr(sexp.first),
            self._expand_list(sexp.second),
        )

    def _emit_module(self):
        self._emit_text("(module")
        self.indent += 4
        # TODO: do these exports in a builtin together
        self._emit_text('  (func $write_i32 (import "" "write_i32") (param i32))')
        self._emit_text(_builtin_types)

        # Start a new lexical frame for the builtins.
        self.lexical_env.append([])

        for blt in _builtins:
            self._emit_text(
                blt.code_templ.format(**blt.code_params),
            )

            # Add the builtin to the lexical environment, in the same order
            # they are enqueued onto the runtime environment in _emit_startfunc.
            self.lexical_env[-1].insert(0, blt.name)

        self._emit_startfunc()

        self.indent -= 4
        self._emit_text(")")

    def _emit_startfunc(self):
        self._emit_line("")
        self._emit_line('(func (export "start") (result i32)')
        self.indent += 4
        self._emit_line("(local $builtins anyref)")

        # Build the builtins environment frame.
        # Each builtin has a corresponding table index which is its position
        # in the list of builtins.
        for i, blt in enumerate(_builtins):
            self._emit_line(
                f"(local.set $builtins (struct.new $PAIR (struct.new $CLOSURE (ref.null $ENV) (i32.const {i})) (local.get $builtins)))"
            )

        self._emit_line("")
        self._emit_line(";; call toplevel user function")
        self._emit_line("(call $user_func_0 (local.get $builtins) (ref.null $ENV))")
        self._emit_line("(i32.const 0)")
        self.indent -= 4
        self._emit_line(")")

    def _emit_line(self, line: str):
        self.stream.write(" " * self.indent + line + "\n")

    def _emit_text(self, text: str):
        for line in text.splitlines():
            self._emit_line(line)


def expr_tree_repr(expr):
    sbuf = StringIO()

    def rec(v, indent):
        prefix = " " * indent
        if v is None:
            return
        match v:
            case Boolean() | Symbol() | Number():
                sbuf.write(f"{prefix}{repr(v)}\n")
            case Pair(first=first, second=second):
                sbuf.write(f"{prefix}Pair\n")
                rec(first, indent + 2)
                rec(second, indent + 2)
            case _:
                raise ExprError("Unexpected type: %s" % type(v))

    rec(expr, 0)
    return sbuf.getvalue()


# Function call ABI / convention in emitted code
# ----------------------------------------------
#
# All functions (built-in and user-defined) have the same signature:
#
#   (func (param anyref) (param (ref null $ENV)) (result anyref))
#
# The first parameter is the argument list (as a PAIR struct), and the second
# parameter is the environment (as an ENV struct). The first argument is
# obtained by casting the first parameter to PAIR and taking its car, etc. These
# functions are emitted to the WAT code and registered in the function table for
# indirect calls.
#
# On entry, each function updates the environment by creating a new ENV struct
# whose parent is the passed-in environment, and whose values are the arguments
# passed in the first parameter.
#
# The run-time representation of a function is as a CLOSURE struct, holding a
# reference to the runtime lexical environment at the time of the function's
# creation, and the function index in the function table.


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
