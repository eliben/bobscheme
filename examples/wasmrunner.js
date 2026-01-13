// Node.js loader and runner for scheme compiled to WebAssembly.
//
// Expects a single argument: the path to a .wasm (binary) file.
// Loads the "start" function and runs it, sending its output to stdout.
//
// The "start" function name is part of an ABI agreement between code emitted
// from wasmcompiler.py and this runner. The other parts are the output
// functions the loaded module imports through 'env'.
const fs = require('fs');
const path = require('path');

(async () => {
    // Usage: node wasmrunner.js <file.wasm>
    const [, , wasmArg] = process.argv;
    if (wasmArg === '-h' || wasmArg === '--help') {
        console.log('Usage: node wasmrunner.js <file.wasm>');
        process.exit(0);
    }

    if (!fs.existsSync(wasmArg)) {
        console.error(`WASM file not found: ${wasmArg}`);
        process.exit(1);
    }

    const bytes = fs.readFileSync(wasmArg);
    let importObject = {
        env: {
            write_char : n => {
                process.stdout.write(String.fromCharCode(n));
            },
            write_i32: n => {
                process.stdout.write(`${n}`);
            }
        }
    };
    let obj = await WebAssembly.instantiate(new Uint8Array(bytes), importObject);
    let start = obj.instance.exports.start;
    start();
})();
