There are three separate test scripts here:

* test_interpreter.py: test the interpreter
* test_vm_compiler.py: test the compiler and VM
* test_barevm.py: test the compiler and barevm

Each of these scripts runs all the test cases in testcases/ and checks for
expected results. The test cases are auto-discovered - .scm files in the
testcases/ dir are assumed to be tests. They are run and the standard out
is compared to the corresponding .exp.txt file. See testcases_utils.py
for more details.

To execute individual testcases for debugging, use the scripts in the
examples/ directory to compile Scheme into bytecode and then run it with a VM.

Note for Windows: to run the barevm test, edit test_barevm.py to point to
the path of the barevm executable (this depends on where you asked CMake
to generate the build directory)

