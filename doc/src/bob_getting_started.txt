==========================
 Getting started with Bob
==========================

:Author: Eli Bendersky
:Contact: eliben@gmail.com
:Updated: 09-Apr-2011

.. contents:: Table of Contents
    :backlinks: none

.. sectnum::


Introduction
************

This document will help you get started using / hacking on Bob. 

What is Bob?
------------

Bob is a suite of implementations of the Scheme language in Python. It currently includes:

* A Scheme interpreter
* An implementation of a stack-based virtual machine called the "Bob VM"
* A compiler from Scheme to Bob VM bytecode
* A serializer and deserializer for Bob VM bytecode

In addition, Bob comes with another version of the VM, implemented in C++. It's called "BareVM". BareVM is an optional component - Bob is fully usable as an implementation of Scheme in pure Python.

Here's a schematic description of these parts:

.. image:: diagrams/bob_toplevel.png
  :align: center

Blue boxes represent components implemented in Python. Orange boxes represent components implemented in C++ for BareVM.

License
-------

Bob is in the public domain. You can see it defined in more formal terms in the ``doc/UNLICENSE`` file, or visit `unlicense.org <http://unlicense.org/>`_.

What is Bob useful for?
-----------------------

Bob is a self-educational project. I find that the best way to understand complex concepts and mechanisms is implementing them from scratch. Working on Bob helped me understand the following concepts much better:

* How to write an interpreter for a complete programming-language. This was the easy part, because I've written a few interepreters before. However, it's important in order to provide a base-line implementation with which the other implementations can be compared.
* How to implement a stack-based virtual machine with its own bytecode. Conceptually, the Bob VM is not much different  from "real" VMs, like the one used by Python itself (to be more exact, the official CPython implementation). It's just much simpler.
* How to compile a high-level programming language into much lower-level VM bytecode.

I learned a lot by working on Bob, and I release its code with the hope that it can help educate other people. Bob is written in Python, which is the closest to executable pseudo-code one can get. The code is relatively compact, clean and well documented.

What Scheme does Bob implement?
-------------------------------

Bob implements a representative subset of the standard R5RS Scheme. The initial aim was to allow implementing all the code from `SICP <http://mitpress.mit.edu/sicp/full-text/book/book.html>`_. For an example of what Bob supports, here is the Y combinator, and an invocation of the factorial function defined by using it::

    (define Y
     (lambda (X)
      ((lambda (procedure)
         (X (lambda (arg) ((procedure procedure) arg))))
       (lambda (procedure)
         (X (lambda (arg) ((procedure procedure) arg)))))))
    
    (define F*
     (lambda (func-arg)
      (lambda (n)
        (if (zero? n)
            1
            (* n (func-arg (- n 1)))))))
    
    (define fact (Y F*))
    (write (fact 8))

In any case, extending Bob to support more functionality from R5RS is very easy. I'm adding features on a per-need basis from time to time.

References
----------

Bob doesn't contain too many original ideas - it's mostly based on earlier work. I've done a lot of reading on this subject, both in books and online, but the two foremost resources that stand out are the following books:

* [`SICP <http://mitpress.mit.edu/sicp/full-text/book/book.html>`_] *Structure and Implementation of Computer Programs, 2nd edition*, by Hal Abelson and Gerald Jay Sussman. The Bob interpreter is based on the *meta-circular interpreter* presented in this book.
* [`PAIP <http://norvig.com/paip.html>`_] *Paradigms of Artificial Intelligence Programming*, by Peter Norvig. The Bob VM is inspired by the *hyperbolical stack machine* presented by Norvig in chapter 23.

In addition, the mainline implementation of Python (CPython) has served as an inspiration for a few parts of Bob. The compilation + VM flow described by the diagram in section `What is Bob?`_ is very similar to how CPython works under the hood.

Design philosophy
-----------------

The single guiding design philosophy of Bob is *clarity and simplicity*. There's a huge amount of optimizations I can envision applying to Bob to make it more compact and faster. I avoided these optimizations on purpose. A sad inherent conflict in software is that optimization and clarity are almost always at odds - you can gain one, but usually sacrifice the other in the process. 

The mechanisms Bob implements are complex enough even without optimizations. Therefore, I aimed to create a design that's as simple as possible, and implement it with code that's as simple and clean as possible. 

Getting started with Bob
************************

Documentation
-------------

The Bob documentation is collected in the ``doc`` directory. It's written in plain text files, in reStructuredText format and is converted to HTML with the ``doc/src/__gen_html.py`` script. At the moment, the following documents are available:

* ``doc/bob_getting_started.html``: this document
* ``doc/bob_bytecode_reference.html``: reference of the Bob bytecode 

However, the best documentation of Bob is its source code. Bob is written in executable pseudo-code (Python) and the internal documentation strings and comments are relatively comprehensive. If you feel something is missing or some part of Bob's code isn't documented well enough, don't hesitate to drop me an email - I will gladly improve it.

Dependencies
------------

Bob is implemented in pure Python and is currently completely self-contained. To run it, you just need to have Python installed. Bob runs with Python 2.6 and later, including 3.x.

Structure of the source tree
----------------------------

* ``doc``: Documentation.

* ``bob``: The Python source code implementing Bob.

* ``examples``: Some examples of using Bob in a script.

* ``tests_full``: Tests exercising Bob's code by running complete Scheme programs and making sure they produce the expected output.

* ``barevm``: BareVM source code and unit tests. See the `BareVM`_ section for more details.

* ``experimental``: Here be dragons! Experimental code not currently suitable for use.

Running examples
----------------

Bob is implemented as a set of Python modules that can be imported and used from any Python script. The ``examples`` directory contains a few very simple scripts that show how to use Bob. 

Running the interpreter in interactive mode
===========================================

Bob contains an implementation of a Scheme interpreter. This interpreter has a very rudimentary interactive mode, mimicking a standard Scheme REPL (Read-Eval-Print loop). You can see how to invoke it by running the ``interactive.py`` example. Here's a sample interaction::

  .../examples> interactive.py
  Interactive Bob interpreter. Type a Scheme expression or 'quit'
  [bob] >> 1
  : 1
  [bob] >> (+ 2 1)
  : 3
  [bob] >> (define (foo a b) (+ a b 10))
  [bob] >> (foo 6 7)
  : 23
  [bob] >>

Compiling Scheme into Bob VM bytecode and executing the bytecode
================================================================

The ``examples`` directory contains a simple Scheme program in ``simple_func.scm``::

  (define (func a b)
    (lambda (x)
      (+ a b x)))

  (write ((func 4 5) 10))

The script ``compile_file.py`` invokes the Bob compiler to compile Scheme into Bob VM bytecode and serialize this bytecode into a file with extension ``.bobc``. The script ``run_compiled.py`` loads Bob VM bytecode from ``.bobc`` files and runs it on the Bob VM. Here's a simple interaction with these scripts::

  .../examples> compile_file.py simple_func.scm
  Output file created: simple_func.bobc

  .../examples> run_compiled.py simple_func.bobc
  19

Running tests
-------------

The ``tests_full`` directory contains a test suite for running complete Scheme programs with Bob. To make sure Bob runs correctly, execute both ``test_interpreter.py`` and ``test_vm_compiler.py``, and see that no errors are reported. 

The file ``testcases_utils.py`` contains the simple testing infrastructure code used to implement the actual tests, and the ``testcases`` directory contains complete Scheme programs with expected output files. New test cases added to this directory will be automatically "discovered" and run by the testing scripts.

BareVM
******

BareVM is an implementation of the Bob VM in C++. It should work as a drop-in replacement for the Python Bob VM, with one small difference:

* Bob VM is implemented as a library of Python classes. The ``examples/run_compiled.py`` example shows how to instantiate a ``Deserializer`` object followed by a ``BobVM`` object to execute VM bytecode from a ``.bobc`` file.
* BareVM compiles to a standalone executable that loads a ``.bobc`` file, deserializes it and executes the bytecode in it.

Why is BareVM interesting? Python is a powerful programming language - sometimes *too* powerful. In particular, when implementing a virtual machine, Python makes the task relatively easy. Its powerful object system with duck typing, reflection capabilities and built-in garbage collection is something most Python programmers take for granted.

And yet, most real-world VMs are implemented in C or C++, since a VM is one of those programs which are never fast enough. Compared to Python, these are low-level languages requiring much more of the implementation to be explicit. BareVM was created as an exercise in VM implementation in a low-level language. One of its most interesting features is a complete mark and sweep garbage collector. BareVM is also significantly faster than the Python Bob VM. Without any particular optimization efforts (following roughtly the same implementation logic as in the Python code), the performance is 5x  on many benchmarks.


Structure of the ``barevm`` sub-tree
------------------------------------

* ``CMakeLists.txt``: Main CMake definition file for Barevm
* ``maketags.sh``: Generate source code tags for Linux code editors
* ``src/``: BareVM source code
* ``tests_unit/``: BareVM unit tests, including a packaged version of `gtest <http://code.google.com/p/googletest/>`_.

Building
--------

BareVM uses `CMake <http://www.cmake.org>`_ for its build:

* On Linux, run ``cmake .`` in the BareVM ``src`` directory, followed by ``make``. This creates two executables in the same directory: ``barevm`` is the BareVM executable, ``barevm_unittest`` runs the unit tests.
* On Windows, run the ``cmake`` GUI, configure the project and generate a Microsoft Visual Studio solution. Build the project from the solution.

Running and testing
-------------------

The main BareVM driver (implemented in ``src/main.cpp``) is very simple - it expects a ``.bobc`` file (containing serialized Bob bytecode) as a single argument, runs the bytecode and displays the output. It can be used as a drop-in replacement for ``examples/run_compiled.py``.

BareVM comes with a few simple unit tests which are run by executing ``src/barevm_unittest``, once this file is generated by the compiler.

The most comprehensive tests on BareVM are done by running the full tests. ``tests_full/test_barevm.py`` uses the Python Bob compiler from Scheme to bytecode, in unison with BareVM to execute the tests, thus testing BareVM on the whole set of full testcases. By default, the path to barevm in ``tests_full/test_barevm.py`` points to the executable generated on Linux. If you want to run these tests on Windows or move the executable to another location, modify the path accordingly.



