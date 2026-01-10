#!/usr/bin/env python3
# -------------------------------------------------------------------------------
# bob: tests_full/test_wasm_compiler.py
#
# Run the full tests for the WASM compiler of Bob by compiling Scheme code to
# WAT, converting to WASM with wasm-tools, and executing via Node.js using
# examples/wasmrunner.js. This mirrors examples/compile_to_wasm.py but plugs
# into the common testcases harness.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
# -------------------------------------------------------------------------------
import io
import shutil
import subprocess
import tempfile
from pathlib import Path

from testcases_utils import run_tests

from bob.bobparser import BobParser
from bob.wasmcompiler import WasmCompiler


# Locate external tools required for running the WASM backend end-to-end.
WASM_TOOLS = shutil.which("wasm-tools")
NODE = shutil.which("node")


def wasm_compiler_runner(code: str, ostream: io.StringIO) -> None:
    """Compile Scheme code to WAT, then to WASM, then run via Node.

    Any stdout produced by the program (via the Scheme 'write' builtin) is
    captured and written into 'ostream'. Errors are caught and surfaced as
    output so the harness can report them per-test without aborting the run.
    """
    parser = BobParser()
    exprs = parser.parse(code)

    wat_stream = io.StringIO()
    WasmCompiler(wat_stream).compile(exprs)
    wat_text = wat_stream.getvalue()

    with tempfile.TemporaryDirectory() as tmpd:
        tmpdir = Path(tmpd)
        wat_path = tmpdir / "program.wat"
        wasm_path = tmpdir / "program.wasm"
        wat_path.write_text(wat_text)

        # Convert WAT -> WASM using wasm-tools CLI (like examples/compile_to_wasm.py).
        parse_proc = subprocess.run(
            [WASM_TOOLS, "parse", str(wat_path), "-o", str(wasm_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if parse_proc.returncode != 0:
            ostream.write(f"[wasm-tools parse failed]\n{parse_proc.stderr}\n")
            return

        # Execute with Node.js runner to produce stdout for comparison.
        runner_js = Path(__file__).resolve().parents[1] / "examples" / "wasmrunner.js"
        run_proc = subprocess.run(
            [NODE, str(runner_js), str(wasm_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if run_proc.returncode != 0:
            ostream.write(f"[node execution failed]\n{run_proc.stderr}\n")
            return

        ostream.write(run_proc.stdout)


supported_tests = {
    "abasic1",
    "abasic2",
    "abasic3",
    "func1",
    "func2",
    "vars1",
    "cond1",
    "lists1",
    "lists2",
    "lists3",
    "symbolic1",
    # "symbolic-diff",
    "numeric1",
    "builtins",
}

run_tests(runner=wasm_compiler_runner, testnames=supported_tests)
