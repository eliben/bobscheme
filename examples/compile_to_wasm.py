#!/usr/bin/env python3
import argparse
import io
import os
import subprocess
import tempfile
import shutil
import sys
from pathlib import Path

from bob.bobparser import BobParser
from bob.expr import expr_repr
from bob.wasmcompiler import WasmCompiler, expr_tree_repr

default_scheme_program = """
(define (map proc lst)
    (if (null? lst)
        '()
        (cons (proc (car lst)) (map proc (cdr lst)))))

(define (filter proc lst)
    (cond 
        ((null? lst) '())
        ((proc (car lst)) (cons (car lst) (filter proc (cdr lst))))
        (else (filter proc (cdr lst)))))

(write (map (lambda (x) (* x x)) '(1 2 3 4)))
(write (filter (lambda (x) (> x 2)) '(1 2 3 4)))


"""


def main():
    ap = argparse.ArgumentParser(
        description="Compile a Scheme snippet or file to WAT, optionally execute via Node"
    )
    ap.add_argument(
        "scheme_file",
        nargs="?",
        help="Path to a Scheme source file. If omitted, uses an internal sample program.",
    )
    ap.add_argument(
        "--execute",
        "-x",
        action="store_true",
        help="Execute compiled WASM via Node.js after compilation",
    )
    args = ap.parse_args()

    if args.scheme_file:
        try:
            scmprog = Path(args.scheme_file).read_text()
        except OSError as e:
            raise SystemExit(f"Failed to read Scheme file '{args.scheme_file}': {e}")
    else:
        scmprog = default_scheme_program

    parser = BobParser()
    exprs = parser.parse(scmprog)

    # Create a StringIO to hold the WAT output
    wat_stream = io.StringIO()

    compiler = WasmCompiler(wat_stream)
    compiler.compile(exprs)
    wat_text = wat_stream.getvalue()

    if not args.execute:
        print(wat_text)
        return

    # Ensure required tools exist
    wasm_tools = shutil.which("wasm-tools")
    node = shutil.which("node")
    if wasm_tools is None:
        raise SystemExit(
            "wasm-tools not found in PATH. Install bytecodealliance wasm-tools."
        )
    if node is None:
        raise SystemExit(
            "node not found in PATH. Install Node.js to run wasmrunner.js."
        )

    # Write WAT to a temp file and parse to WASM
    with tempfile.TemporaryDirectory(delete=False) as tmpd:
        wat_path = Path(tmpd) / "program.wat"
        wasm_path = Path(tmpd) / "program.wasm"
        wat_path.write_text(wat_text)
        print(f"Wrote WAT to {wat_path}", file=sys.stderr)

        # Convert WAT -> WASM
        try:
            subprocess.run(
                [wasm_tools, "parse", str(wat_path), "-o", str(wasm_path)], check=True
            )
        except subprocess.CalledProcessError as e:
            raise SystemExit(f"wasm-tools parse failed: {e}")
        print(f"Compiled WASM to {wasm_path}", file=sys.stderr)
        print("------", file=sys.stderr)

        # Run via Node wasmrunner.js, printing stdout
        runner_js = Path(__file__).with_name("wasmrunner.js")
        try:
            subprocess.run([node, str(runner_js), str(wasm_path)], check=True)
        except subprocess.CalledProcessError as e:
            raise SystemExit(f"Node execution failed: {e}")


if __name__ == "__main__":
    main()
