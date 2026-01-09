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
        nestedlist = make_nested_pairs(*exprlist)
        # print(expr_tree_repr(nestedlist)) # TODO
        tpl = self._expand_block(nestedlist)
        # print("----")
        # print(expr_tree_repr(tpl))
        self._emit_module(tpl)

    def _expand_block(self, exprlist):
        # Sequence of expressions. We iterate over it from the last to the first
        # to build a nested BEGIN structure (in 'result'). For each expression,
        # if it's a definition, _expand_definition expands it and returns the
        # defined names, which we prepend to the names list.
        if not isinstance(exprlist, Pair):
            return self._expand_expr(exprlist)

        result = None
        names = None
        for expr in reverse_iter_pairs(exprlist):
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
            args = Pair(None, args)

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
                case "cond":
                    return self._expand_expr(convert_cond_to_ifs(expr))
                case "begin":
                    return self._expand_block(begin_actions(expr))
                case "set!":
                    return make_assignment(
                        assignment_variable(expr),
                        self._expand_expr(assignment_value(expr)),
                    )

        return self._expand_list(expr)

    def _expand_list(self, expr):
        # Recurisve list expansion: expand head with _expand_expr, and tail
        # with _expand_list.
        if expr is None:
            return None
        assert isinstance(expr, Pair)
        return Pair(
            self._expand_expr(expr.first),
            self._expand_list(expr.second),
        )

    def _emit_module(self, expr):
        self._emit_text("(module")
        self.indent += 4
        self._emit_text(_imports)
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

        # Emit user code, recursively, starting with the toplevel expression.
        # These are emitted into their own streams stored in self.user_funcs.
        self._emit_proc(expr)

        # Emit all user-defined functions to the output stream.
        for i, func_stream in enumerate(self.user_funcs):
            self._emit_text(func_stream.getvalue())

        # Emit table and element section.
        self._emit_line("")
        self._emit_line(
            "(table {} funcref)".format(len(_builtins) + len(self.user_funcs))
        )
        elem_index = 0
        for blt in _builtins:
            name = blt.name
            if blt.code_params.get("NAME", False):
                name = blt.code_params["NAME"]
            self._emit_line(f"(elem (i32.const {elem_index}) ${name})")
            elem_index += 1
        for i in range(len(self.user_funcs)):
            self._emit_line(f"(elem (i32.const {elem_index}) $user_func_{i})")
            elem_index += 1

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
        self._emit_line("drop")
        self._emit_line("(i32.const 0)")
        self.indent -= 4
        self._emit_line(")")

    def _emit_proc(self, expr):
        # Create a new stream for this function's body; the current one will
        # be restored at the end of this function.
        saved_stream = self.stream
        saved_indent = self.indent
        saved_tailcall_pos = self.tailcall_pos

        self.stream = StringIO()
        self.indent = 0
        self.tailcall_pos = 0

        func_idx = len(self.user_funcs)
        self.user_funcs.append(self.stream)

        self._emit_line(
            f"(func $user_func_{func_idx} (param $arg anyref) (param $env (ref null $ENV)) (result anyref)"
        )
        self.indent += 4
        self._emit_line("(local $clostemp (ref null $CLOSURE))")
        self._emit_line(";; prologue: env = new ENV(env, args)")
        self._emit_line(
            "(local.set $env (struct.new $ENV (local.get $env) (local.get $arg)))"
        )
        self._emit_expr(expr)
        self.indent -= 4
        self._emit_line(")")
        self.indent = saved_indent
        self.stream = saved_stream
        self.tailcall_pos = saved_tailcall_pos
        return func_idx

    def _emit_expr(self, expr):
        if expr is None:
            self._emit_line("(ref.null any)")
        elif is_self_evaluating(expr):
            self._emit_constant(expr)
        elif isinstance(expr, Symbol):
            self._emit_var(expr.value)
            self._emit_line(";; load variable value from its cons cell")
            self._emit_line("(struct.get $PAIR 0 (ref.cast (ref $PAIR)))")
        elif is_assignment(expr):
            name = assignment_variable(expr).value
            self._emit_line(f";; set! '{name}' = expression")
            self._emit_var(name)
            self._emit_line("(ref.cast (ref $PAIR))")
            self.tailcall_pos += 1
            self._emit_expr(assignment_value(expr))
            self.tailcall_pos -= 1
            self._emit_line("(struct.set $PAIR 0)")
            self._emit_line("(ref.null any)")
        elif is_quoted(expr):
            qval = text_of_quotation(expr)
            self._emit_constant(qval)
        elif is_begin(expr):
            self._emit_line(";; begin")
            self.tailcall_pos += 1
            self._emit_expr(begin_actions(expr).first)
            self.tailcall_pos -= 1
            self._emit_line("drop")
            self._emit_expr(begin_actions(expr).second)
        elif is_lambda(expr):
            self._emit_line(";; lambda expression")
            # Add the lambda parameters to the lexical environment and
            # emit the body into a separate function.
            frame = [p.value for p in iter_pairs(lambda_parameters(expr))]
            self.lexical_env.append(frame)
            func_idx = self._emit_proc(lambda_body(expr))
            self.lexical_env.pop()
            # func_idx is the index of the function in the user_funcs list.
            # Since we emit builtins first, we need to offset it by
            # len(_builtins).
            elem_idx = func_idx + len(_builtins)
            self._emit_line(
                f"(struct.new $CLOSURE (local.get $env) (i32.const {elem_idx}))"
            )
        elif is_if(expr):
            self._emit_line(";; ifelse condition")
            self.tailcall_pos += 1
            self._emit_expr(if_predicate(expr))
            self.tailcall_pos -= 1
            self._emit_line("ref.is_null")
            self._emit_line("if (result anyref)")
            self.indent += 4
            self._emit_line(";; else branch")
            self._emit_expr(if_alternative(expr))
            self.indent -= 4
            self._emit_line("else")
            self.indent += 4
            self._emit_line(";; then branch")
            self._emit_expr(if_consequent(expr))
            self.indent -= 4
            self._emit_line("end")

        elif is_application(expr):
            self.tailcall_pos += 1
            self._emit_list(application_operands(expr))
            self._emit_expr(application_operator(expr))
            self.tailcall_pos -= 1
            self._emit_line(";; call function")
            self._emit_line("ref.cast (ref $CLOSURE)")
            self._emit_line("local.tee $clostemp")
            self._emit_line("struct.get $CLOSURE 0  ;; get env")
            self._emit_line("local.get $clostemp")
            self._emit_line("struct.get $CLOSURE 1  ;; get function index")
            self._emit_line(";; stack for call: [args] [env] [func idx]")

            if self.tailcall_pos == 0:
                self._emit_line("return_call_indirect (type $FUNC)")
            else:
                self._emit_line("call_indirect (type $FUNC)")
        else:
            raise ExprError(f"Unexpected expression {expr}")

    def _emit_constant(self, expr):
        match expr:
            case None:
                self._emit_line("(ref.null any)")
            case Number(value=n):
                self._emit_line(f"(ref.i31 (i32.const {n}))")
            case Boolean(value=b):
                if b:
                    self._emit_line("(struct.new $BOOL (i32.const 1))")
                else:
                    self._emit_line("(struct.new $BOOL (i32.const 0))")
            case Pair(first=first, second=second):
                self._emit_line(";; cons cell for constant")
                self._emit_constant(first)
                self._emit_constant(second)
                self._emit_line("struct.new $PAIR")
            case _:
                raise ExprError("Unexpected constant type: %s" % type(expr))

    def _emit_list(self, lst):
        if isinstance(lst, Pair):
            self._emit_expr(lst.first)
            self._emit_list(lst.second)
            self._emit_line("struct.new $PAIR")
        else:
            self._emit_line("(ref.null any)")

    def _emit_var(self, name: str):
        # What it leaves on the stack is not the value of the variable, but
        # the cons cell holding it; the caller is responsible for loading
        # the value from the cell, or for mutating it if needed (in set!).
        frame_index = len(self.lexical_env) - 1
        self._emit_line(f";; lookup variable '{name}' in lexical environment")
        self._emit_line("local.get $env")
        while frame_index >= 0:
            try:
                var_index = self.lexical_env[frame_index].index(name)
                # Get list of args from this env frame.
                self._emit_line(f";; found in frame, at index {var_index}")
                self._emit_line("struct.get $ENV 1")
                for _ in range(var_index):
                    self._emit_line("(struct.get $PAIR 1 (ref.cast (ref $PAIR)))")
                return
            except ValueError:
                # Not found in this frame; go to parent.
                self._emit_line("struct.get $ENV 0 ;; get parent env")
                frame_index -= 1

        raise ValueError(f"Variable '{name}' not found in lexical environment")

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


_imports = r"""
(import "env" "write_char" (func $write_char (param i32)))
(import "env" "write_i32" (func $write_i32 (param i32)))
"""

_builtin_types = r"""
;; PAIR holds the car and cdr of a cons cell.
(type $PAIR (struct (field (mut anyref)) (field (mut anyref))))

;; BOOL represents a Scheme boolean. zero -> false, nonzero -> true.
(type $BOOL (struct (field i32)))

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

_write_code = r"""
;; The emit* functions use the imported write_char and write_i32 host functions.
(func $emit (param $c i32)
    (call $write_char (local.get $c))
)

(func $emit_lparen  (call $emit (i32.const 40))) ;; '('
(func $emit_rparen  (call $emit (i32.const 41))) ;; ')'
(func $emit_space   (call $emit (i32.const 32))) ;; ' '
(func $emit_newline (call $emit (i32.const 10))) ;; '\n'

(func $emit_bool (param $b (ref $BOOL))
    (call $emit (i32.const 35)) ;; '#'
    (if (i32.eqz (struct.get $BOOL 0 (local.get $b)))
        (then (call $emit (i32.const 102))) ;; 'f'
        (else (call $emit (i32.const 116))) ;; 't'
    )
)

(func $emit_value (param $v anyref)
    ;; nil
    (if (ref.is_null (local.get $v))
        (then
            (call $emit_lparen)
            (call $emit_rparen)
            (return)
        )
    )

    ;; integer
    (if (ref.test (ref i31) (local.get $v))
        (then
            (call $write_i32 (i31.get_s (ref.cast (ref i31) (local.get $v))))
            (return)
        )
    )

    ;; bool
    (if (ref.test (ref $BOOL) (local.get $v))
        (then
            (call $emit_bool (ref.cast (ref $BOOL) (local.get $v)))
            (return)
        )
    )

    ;; pair
    (if (ref.test (ref $PAIR) (local.get $v))
        (then
            (call $emit_pair (ref.cast (ref $PAIR) (local.get $v)))
            (return)
        )
    )

    ;; unknown type: emit '?'
    (call $emit (i32.const 63))
)

(func $emit_pair (param $p (ref $PAIR))
    (local $cur (ref null $PAIR))
    (local $cdr anyref)

    (call $emit_lparen)
    (local.set $cur (local.get $p))

    (loop $loop (block $breakloop
        ;; print car
        (call $emit_value (struct.get $PAIR 0 (local.get $cur)))

        (local.set $cdr (struct.get $PAIR 1 (local.get $cur)))
        
        ;; end of list?
        (br_if $breakloop (ref.is_null (local.get $cdr)))

        ;; cdr is another pair, continue loop
        (if (ref.test (ref $PAIR) (local.get $cdr))
            (then
                (call $emit_space)
                (local.set $cur (ref.cast (ref $PAIR) (local.get $cdr)))
                br $loop
            )
            (else
                ;; cdr is not a pair, print dot and the cdr value, then end
                (call $emit (i32.const 32)) ;; space
                (call $emit (i32.const 46)) ;; '.'
                (call $emit (i32.const 32)) ;; space
                (call $emit_value (local.get $cdr))
                br $breakloop
            )
        )
    ))

    (call $emit_rparen)
)

(func $write (param $arg anyref) (param $env (ref null $ENV)) (result anyref)
    (call $emit_value (struct.get $PAIR 0 (ref.cast (ref $PAIR) (local.get $arg))))
    (call $emit (i32.const 10)) ;; newline
    (ref.null any)
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
_register_builtin("write", _write_code, {})
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
