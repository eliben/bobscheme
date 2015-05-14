========================
 Bob bytecode reference
========================

:Author: Eli Bendersky
:Contact: eliben@gmail.com
:Updated: 09-Apr-2011

.. contents:: Table of Contents
    :backlinks: none

.. sectnum::


Introduction
************

This document describes the bytecode that's produced by the Bob compiler from Scheme source and can be executed on the Bob VM. This bytecode can exist as either an in-memory object, or serialized to a string/file. It is suitable for generation from a compiler, not by human manual construction - there's no corresponding human-readable "assembly language" syntax that can be used to write the bytecode manually.

The Bob stack-based VM
**********************

The Bob VM is a `stack machine <http://en.wikipedia.org/wiki/Stack_machine>`_, implemented with two stacks:

* The explicit value stack, called just *the stack* throughout the documentation and code. Used to load and store values of variables from and to the symbol table ("environment"), save temporary values, pass arguments to functions and return values from functions. 
* An implicit "frame stack" used by the ``CALL`` and ``RETURN`` instructions to implement function calls, by saving the current VM state ("execution frame") before calling a function and restoring it when returning from a function.

Bytecode instructions
*********************

The instructions supported by the Bob bytecode are summarized in the following table.

.. list-table:: 
   :widths: 15 10 30
   :header-rows: 1

   * - Opcode
     - Argument
     - Description
   * - CONST
     - value
     - Push constant value onto stack
   * - LOADVAR
     - symbol
     - Push value of the symbol onto stack
   * - STOREVAR
     - symbol
     - Pop value from stack and make it the symbol's value
   * - DEFVAR
     - symbol 
     - Pop value from stack and define a new symbol to hold it
   * - FUNCTION
     - code object
     - Make closure from the code object and the current environment, and push it onto stack
   * - POP
     -  
     - Pop value from stack and discard it
   * - FJUMP
     - label 
     - Pop value from stack. If it's false, jump to label
   * - JUMP
     - label 
     - Jump to label
   * - RETURN
     - 
     - Return from current function 
   * - CALL
     - num 
     - Pop a function value (closure) from stack, and call it. *num* is the amount of arguments passed to the function, currently on top of stack.

``bob/bytecode.py`` defines the opcodes, as well as the key ``Instruction`` and ``CodeObject`` classes which act as simple containers for the bytecode. 
The core of the VM implementation is less than 200 lines of commented Python code in ``bob/vm.py``. 

Serialization
*************

To allow interoperability between different implementations of the VM, Bob supports serialization and de-serialization of bytecode into a platform independent binary string (which can be saved in a file or transmitted over a socket). The serialization scheme was inspired by Python's ``marshal`` module and is implemented by the classes ``Serializer`` and ``Deserializer`` in ``bob/bytecode.py``.



