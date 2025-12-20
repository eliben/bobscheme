Bob is a suite of implementations of the Scheme language in Python and C++. It currently
includes:

* A Scheme interpreter
* An implementation of a stack-based virtual machine called the "Bob VM"
* A compiler from Scheme to Bob VM bytecode
* A serializer and deserializer for Bob VM bytecode

In addition, Bob comes with another version of the VM, implemented in C++. It's
called "BareVM". Bob itself is a fully usable implementation of Scheme in pure
Python; BareVM is an optional component.

There's some documentation in the ``doc/`` directory in the source.
[Getting Started](https://github.com/eliben/bobscheme/blob/master/doc/bob_getting_started.rst).

The following diagram describes Bob:

<p align="center">
  <img alt="Bob diagram" src="doc/diagrams/bob_toplevel.png" />
</p>
