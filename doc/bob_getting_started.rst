<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="generator" content="Docutils 0.6: http://docutils.sourceforge.net/" />
<title>Getting started with Bob</title>
<meta name="author" content="Eli Bendersky" />
<style type="text/css">

/*
:Author: David Goodger (goodger@python.org)
:Id: $Id: html4css1.css 5951 2009-05-18 18:03:10Z milde $
:Copyright: This stylesheet has been placed in the public domain.

Default cascading style sheet for the HTML output of Docutils.

See http://docutils.sf.net/docs/howto/html-stylesheets.html for how to
customize this style sheet.
*/

/* used to remove borders from tables and images */
.borderless, table.borderless td, table.borderless th {
  border: 0 }

table.borderless td, table.borderless th {
  /* Override padding for "table.docutils td" with "! important".
     The right padding separates the table cells. */
  padding: 0 0.5em 0 0 ! important }

.first {
  /* Override more specific margin styles with "! important". */
  margin-top: 0 ! important }

.last, .with-subtitle {
  margin-bottom: 0 ! important }

.hidden {
  display: none }

a.toc-backref {
  text-decoration: none ;
  color: black }

blockquote.epigraph {
  margin: 2em 5em ; }

dl.docutils dd {
  margin-bottom: 0.5em }

/* Uncomment (and remove this text!) to get bold-faced definition list terms
dl.docutils dt {
  font-weight: bold }
*/

div.abstract {
  margin: 2em 5em }

div.abstract p.topic-title {
  font-weight: bold ;
  text-align: center }

div.admonition, div.attention, div.caution, div.danger, div.error,
div.hint, div.important, div.note, div.tip, div.warning {
  margin: 2em ;
  border: medium outset ;
  padding: 1em }

div.admonition p.admonition-title, div.hint p.admonition-title,
div.important p.admonition-title, div.note p.admonition-title,
div.tip p.admonition-title {
  font-weight: bold ;
  font-family: sans-serif }

div.attention p.admonition-title, div.caution p.admonition-title,
div.danger p.admonition-title, div.error p.admonition-title,
div.warning p.admonition-title {
  color: red ;
  font-weight: bold ;
  font-family: sans-serif }

/* Uncomment (and remove this text!) to get reduced vertical space in
   compound paragraphs.
div.compound .compound-first, div.compound .compound-middle {
  margin-bottom: 0.5em }

div.compound .compound-last, div.compound .compound-middle {
  margin-top: 0.5em }
*/

div.dedication {
  margin: 2em 5em ;
  text-align: center ;
  font-style: italic }

div.dedication p.topic-title {
  font-weight: bold ;
  font-style: normal }

div.figure {
  margin-left: 2em ;
  margin-right: 2em }

div.footer, div.header {
  clear: both;
  font-size: smaller }

div.line-block {
  display: block ;
  margin-top: 1em ;
  margin-bottom: 1em }

div.line-block div.line-block {
  margin-top: 0 ;
  margin-bottom: 0 ;
  margin-left: 1.5em }

div.sidebar {
  margin: 0 0 0.5em 1em ;
  border: medium outset ;
  padding: 1em ;
  background-color: #ffffee ;
  width: 40% ;
  float: right ;
  clear: right }

div.sidebar p.rubric {
  font-family: sans-serif ;
  font-size: medium }

div.system-messages {
  margin: 5em }

div.system-messages h1 {
  color: red }

div.system-message {
  border: medium outset ;
  padding: 1em }

div.system-message p.system-message-title {
  color: red ;
  font-weight: bold }

div.topic {
  margin: 2em }

h1.section-subtitle, h2.section-subtitle, h3.section-subtitle,
h4.section-subtitle, h5.section-subtitle, h6.section-subtitle {
  margin-top: 0.4em }

h1.title {
  text-align: center }

h2.subtitle {
  text-align: center }

hr.docutils {
  width: 75% }

img.align-left, .figure.align-left{
  clear: left ;
  float: left ;
  margin-right: 1em }

img.align-right, .figure.align-right {
  clear: right ;
  float: right ;
  margin-left: 1em }

.align-left {
  text-align: left }

.align-center {
  clear: both ;
  text-align: center }

.align-right {
  text-align: right }

/* reset inner alignment in figures */
div.align-right {
  text-align: left }

/* div.align-center * { */
/*   text-align: left } */

ol.simple, ul.simple {
  margin-bottom: 1em }

ol.arabic {
  list-style: decimal }

ol.loweralpha {
  list-style: lower-alpha }

ol.upperalpha {
  list-style: upper-alpha }

ol.lowerroman {
  list-style: lower-roman }

ol.upperroman {
  list-style: upper-roman }

p.attribution {
  text-align: right ;
  margin-left: 50% }

p.caption {
  font-style: italic }

p.credits {
  font-style: italic ;
  font-size: smaller }

p.label {
  white-space: nowrap }

p.rubric {
  font-weight: bold ;
  font-size: larger ;
  color: maroon ;
  text-align: center }

p.sidebar-title {
  font-family: sans-serif ;
  font-weight: bold ;
  font-size: larger }

p.sidebar-subtitle {
  font-family: sans-serif ;
  font-weight: bold }

p.topic-title {
  font-weight: bold }

pre.address {
  margin-bottom: 0 ;
  margin-top: 0 ;
  font: inherit }

pre.literal-block, pre.doctest-block {
  margin-left: 2em ;
  margin-right: 2em }

span.classifier {
  font-family: sans-serif ;
  font-style: oblique }

span.classifier-delimiter {
  font-family: sans-serif ;
  font-weight: bold }

span.interpreted {
  font-family: sans-serif }

span.option {
  white-space: nowrap }

span.pre {
  white-space: pre }

span.problematic {
  color: red }

span.section-subtitle {
  /* font-size relative to parent (h1..h6 element) */
  font-size: 80% }

table.citation {
  border-left: solid 1px gray;
  margin-left: 1px }

table.docinfo {
  margin: 2em 4em }

table.docutils {
  margin-top: 0.5em ;
  margin-bottom: 0.5em }

table.footnote {
  border-left: solid 1px black;
  margin-left: 1px }

table.docutils td, table.docutils th,
table.docinfo td, table.docinfo th {
  padding-left: 0.5em ;
  padding-right: 0.5em ;
  vertical-align: top }

table.docutils th.field-name, table.docinfo th.docinfo-name {
  font-weight: bold ;
  text-align: left ;
  white-space: nowrap ;
  padding-left: 0 }

h1 tt.docutils, h2 tt.docutils, h3 tt.docutils,
h4 tt.docutils, h5 tt.docutils, h6 tt.docutils {
  font-size: 100% }

ul.auto-toc {
  list-style-type: none }

</style>
</head>
<body>
<div class="document" id="getting-started-with-bob">
<h1 class="title">Getting started with Bob</h1>
<table class="docinfo" frame="void" rules="none">
<col class="docinfo-name" />
<col class="docinfo-content" />
<tbody valign="top">
<tr><th class="docinfo-name">Author:</th>
<td>Eli Bendersky</td></tr>
<tr><th class="docinfo-name">Contact:</th>
<td><a class="first last reference external" href="mailto:eliben&#64;gmail.com">eliben&#64;gmail.com</a></td></tr>
<tr class="field"><th class="docinfo-name">Updated:</th><td class="field-body">09-Apr-2011</td>
</tr>
</tbody>
</table>
<div class="contents topic" id="table-of-contents">
<p class="topic-title first">Table of Contents</p>
<ul class="auto-toc simple">
<li><a class="reference internal" href="#introduction" id="id3">1&nbsp;&nbsp;&nbsp;Introduction</a><ul class="auto-toc">
<li><a class="reference internal" href="#what-is-bob" id="id4">1.1&nbsp;&nbsp;&nbsp;What is Bob?</a></li>
<li><a class="reference internal" href="#license" id="id5">1.2&nbsp;&nbsp;&nbsp;License</a></li>
<li><a class="reference internal" href="#what-is-bob-useful-for" id="id6">1.3&nbsp;&nbsp;&nbsp;What is Bob useful for?</a></li>
<li><a class="reference internal" href="#what-scheme-does-bob-implement" id="id7">1.4&nbsp;&nbsp;&nbsp;What Scheme does Bob implement?</a></li>
<li><a class="reference internal" href="#references" id="id8">1.5&nbsp;&nbsp;&nbsp;References</a></li>
<li><a class="reference internal" href="#design-philosophy" id="id9">1.6&nbsp;&nbsp;&nbsp;Design philosophy</a></li>
</ul>
</li>
<li><a class="reference internal" href="#id2" id="id10">2&nbsp;&nbsp;&nbsp;Getting started with Bob</a><ul class="auto-toc">
<li><a class="reference internal" href="#documentation" id="id11">2.1&nbsp;&nbsp;&nbsp;Documentation</a></li>
<li><a class="reference internal" href="#dependencies" id="id12">2.2&nbsp;&nbsp;&nbsp;Dependencies</a></li>
<li><a class="reference internal" href="#structure-of-the-source-tree" id="id13">2.3&nbsp;&nbsp;&nbsp;Structure of the source tree</a></li>
<li><a class="reference internal" href="#running-examples" id="id14">2.4&nbsp;&nbsp;&nbsp;Running examples</a><ul class="auto-toc">
<li><a class="reference internal" href="#running-the-interpreter-in-interactive-mode" id="id15">2.4.1&nbsp;&nbsp;&nbsp;Running the interpreter in interactive mode</a></li>
<li><a class="reference internal" href="#compiling-scheme-into-bob-vm-bytecode-and-executing-the-bytecode" id="id16">2.4.2&nbsp;&nbsp;&nbsp;Compiling Scheme into Bob VM bytecode and executing the bytecode</a></li>
</ul>
</li>
<li><a class="reference internal" href="#running-tests" id="id17">2.5&nbsp;&nbsp;&nbsp;Running tests</a></li>
</ul>
</li>
<li><a class="reference internal" href="#barevm" id="id18">3&nbsp;&nbsp;&nbsp;BareVM</a><ul class="auto-toc">
<li><a class="reference internal" href="#structure-of-the-barevm-sub-tree" id="id19">3.1&nbsp;&nbsp;&nbsp;Structure of the <tt class="docutils literal">barevm</tt> sub-tree</a></li>
<li><a class="reference internal" href="#building" id="id20">3.2&nbsp;&nbsp;&nbsp;Building</a></li>
<li><a class="reference internal" href="#running-and-testing" id="id21">3.3&nbsp;&nbsp;&nbsp;Running and testing</a></li>
</ul>
</li>
</ul>
</div>
<div class="section" id="introduction">
<h1>1&nbsp;&nbsp;&nbsp;Introduction</h1>
<p>This document will help you get started using / hacking on Bob.</p>
<div class="section" id="what-is-bob">
<h2>1.1&nbsp;&nbsp;&nbsp;What is Bob?</h2>
<p>Bob is a suite of implementations of the Scheme language in Python. It currently includes:</p>
<ul class="simple">
<li>A Scheme interpreter</li>
<li>An implementation of a stack-based virtual machine called the &quot;Bob VM&quot;</li>
<li>A compiler from Scheme to Bob VM bytecode</li>
<li>A serializer and deserializer for Bob VM bytecode</li>
</ul>
<p>In addition, Bob comes with another version of the VM, implemented in C++. It's called &quot;BareVM&quot;. BareVM is an optional component - Bob is fully usable as an implementation of Scheme in pure Python.</p>
<p>Here's a schematic description of these parts:</p>
<div align="center" class="align-center"><img alt="diagrams/bob_toplevel.png" class="align-center" src="diagrams/bob_toplevel.png" /></div>
<p>Blue boxes represent components implemented in Python. Orange boxes represent components implemented in C++ for BareVM.</p>
</div>
<div class="section" id="license">
<h2>1.2&nbsp;&nbsp;&nbsp;License</h2>
<p>Bob is in the public domain. You can see it defined in more formal terms in the <tt class="docutils literal">doc/UNLICENSE</tt> file, or visit <a class="reference external" href="http://unlicense.org/">unlicense.org</a>.</p>
</div>
<div class="section" id="what-is-bob-useful-for">
<h2>1.3&nbsp;&nbsp;&nbsp;What is Bob useful for?</h2>
<p>Bob is a self-educational project. I find that the best way to understand complex concepts and mechanisms is implementing them from scratch. Working on Bob helped me understand the following concepts much better:</p>
<ul class="simple">
<li>How to write an interpreter for a complete programming-language. This was the easy part, because I've written a few interepreters before. However, it's important in order to provide a base-line implementation with which the other implementations can be compared.</li>
<li>How to implement a stack-based virtual machine with its own bytecode. Conceptually, the Bob VM is not much different  from &quot;real&quot; VMs, like the one used by Python itself (to be more exact, the official CPython implementation). It's just much simpler.</li>
<li>How to compile a high-level programming language into much lower-level VM bytecode.</li>
</ul>
<p>I learned a lot by working on Bob, and I release its code with the hope that it can help educate other people. Bob is written in Python, which is the closest to executable pseudo-code one can get. The code is relatively compact, clean and well documented.</p>
</div>
<div class="section" id="what-scheme-does-bob-implement">
<h2>1.4&nbsp;&nbsp;&nbsp;What Scheme does Bob implement?</h2>
<p>Bob implements a representative subset of the standard R5RS Scheme. The initial aim was to allow implementing all the code from <a class="reference external" href="http://mitpress.mit.edu/sicp/full-text/book/book.html">SICP</a>. For an example of what Bob supports, here is the Y combinator, and an invocation of the factorial function defined by using it:</p>
<pre class="literal-block">
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
</pre>
<p>In any case, extending Bob to support more functionality from R5RS is very easy. I'm adding features on a per-need basis from time to time.</p>
</div>
<div class="section" id="references">
<h2>1.5&nbsp;&nbsp;&nbsp;References</h2>
<p>Bob doesn't contain too many original ideas - it's mostly based on earlier work. I've done a lot of reading on this subject, both in books and online, but the two foremost resources that stand out are the following books:</p>
<ul class="simple">
<li>[<a class="reference external" href="http://mitpress.mit.edu/sicp/full-text/book/book.html">SICP</a>] <em>Structure and Implementation of Computer Programs, 2nd edition</em>, by Hal Abelson and Gerald Jay Sussman. The Bob interpreter is based on the <em>meta-circular interpreter</em> presented in this book.</li>
<li>[<a class="reference external" href="http://norvig.com/paip.html">PAIP</a>] <em>Paradigms of Artificial Intelligence Programming</em>, by Peter Norvig. The Bob VM is inspired by the <em>hyperbolical stack machine</em> presented by Norvig in chapter 23.</li>
</ul>
<p>In addition, the mainline implementation of Python (CPython) has served as an inspiration for a few parts of Bob. The compilation + VM flow described by the diagram in section <a class="reference internal" href="#what-is-bob">What is Bob?</a> is very similar to how CPython works under the hood.</p>
</div>
<div class="section" id="design-philosophy">
<h2>1.6&nbsp;&nbsp;&nbsp;Design philosophy</h2>
<p>The single guiding design philosophy of Bob is <em>clarity and simplicity</em>. There's a huge amount of optimizations I can envision applying to Bob to make it more compact and faster. I avoided these optimizations on purpose. A sad inherent conflict in software is that optimization and clarity are almost always at odds - you can gain one, but usually sacrifice the other in the process.</p>
<p>The mechanisms Bob implements are complex enough even without optimizations. Therefore, I aimed to create a design that's as simple as possible, and implement it with code that's as simple and clean as possible.</p>
</div>
</div>
<div class="section" id="id2">
<h1>2&nbsp;&nbsp;&nbsp;Getting started with Bob</h1>
<div class="section" id="documentation">
<h2>2.1&nbsp;&nbsp;&nbsp;Documentation</h2>
<p>The Bob documentation is collected in the <tt class="docutils literal">doc</tt> directory. It's written in plain text files, in reStructuredText format and is converted to HTML with the <tt class="docutils literal">doc/src/__gen_html.py</tt> script. At the moment, the following documents are available:</p>
<ul class="simple">
<li><tt class="docutils literal">doc/bob_getting_started.html</tt>: this document</li>
<li><tt class="docutils literal">doc/bob_bytecode_reference.html</tt>: reference of the Bob bytecode</li>
</ul>
<p>However, the best documentation of Bob is its source code. Bob is written in executable pseudo-code (Python) and the internal documentation strings and comments are relatively comprehensive. If you feel something is missing or some part of Bob's code isn't documented well enough, don't hesitate to drop me an email - I will gladly improve it.</p>
</div>
<div class="section" id="dependencies">
<h2>2.2&nbsp;&nbsp;&nbsp;Dependencies</h2>
<p>Bob is implemented in pure Python and is currently completely self-contained. To run it, you just need to have Python installed. Bob runs with Python 2.6 and later, including 3.x.</p>
</div>
<div class="section" id="structure-of-the-source-tree">
<h2>2.3&nbsp;&nbsp;&nbsp;Structure of the source tree</h2>
<ul class="simple">
<li><tt class="docutils literal">doc</tt>: Documentation.</li>
<li><tt class="docutils literal">bob</tt>: The Python source code implementing Bob.</li>
<li><tt class="docutils literal">examples</tt>: Some examples of using Bob in a script.</li>
<li><tt class="docutils literal">tests_full</tt>: Tests exercising Bob's code by running complete Scheme programs and making sure they produce the expected output.</li>
<li><tt class="docutils literal">barevm</tt>: BareVM source code and unit tests. See the <a class="reference internal" href="#barevm">BareVM</a> section for more details.</li>
<li><tt class="docutils literal">experimental</tt>: Here be dragons! Experimental code not currently suitable for use.</li>
</ul>
</div>
<div class="section" id="running-examples">
<h2>2.4&nbsp;&nbsp;&nbsp;Running examples</h2>
<p>Bob is implemented as a set of Python modules that can be imported and used from any Python script. The <tt class="docutils literal">examples</tt> directory contains a few very simple scripts that show how to use Bob.</p>
<div class="section" id="running-the-interpreter-in-interactive-mode">
<h3>2.4.1&nbsp;&nbsp;&nbsp;Running the interpreter in interactive mode</h3>
<p>Bob contains an implementation of a Scheme interpreter. This interpreter has a very rudimentary interactive mode, mimicking a standard Scheme REPL (Read-Eval-Print loop). You can see how to invoke it by running the <tt class="docutils literal">interactive.py</tt> example. Here's a sample interaction:</p>
<pre class="literal-block">
.../examples&gt; interactive.py
Interactive Bob interpreter. Type a Scheme expression or 'quit'
[bob] &gt;&gt; 1
: 1
[bob] &gt;&gt; (+ 2 1)
: 3
[bob] &gt;&gt; (define (foo a b) (+ a b 10))
[bob] &gt;&gt; (foo 6 7)
: 23
[bob] &gt;&gt;
</pre>
</div>
<div class="section" id="compiling-scheme-into-bob-vm-bytecode-and-executing-the-bytecode">
<h3>2.4.2&nbsp;&nbsp;&nbsp;Compiling Scheme into Bob VM bytecode and executing the bytecode</h3>
<p>The <tt class="docutils literal">examples</tt> directory contains a simple Scheme program in <tt class="docutils literal">simple_func.scm</tt>:</p>
<pre class="literal-block">
(define (func a b)
  (lambda (x)
    (+ a b x)))

(write ((func 4 5) 10))
</pre>
<p>The script <tt class="docutils literal">compile_file.py</tt> invokes the Bob compiler to compile Scheme into Bob VM bytecode and serialize this bytecode into a file with extension <tt class="docutils literal">.bobc</tt>. The script <tt class="docutils literal">run_compiled.py</tt> loads Bob VM bytecode from <tt class="docutils literal">.bobc</tt> files and runs it on the Bob VM. Here's a simple interaction with these scripts:</p>
<pre class="literal-block">
.../examples&gt; compile_file.py simple_func.scm
Output file created: simple_func.bobc

.../examples&gt; run_compiled.py simple_func.bobc
19
</pre>
</div>
</div>
<div class="section" id="running-tests">
<h2>2.5&nbsp;&nbsp;&nbsp;Running tests</h2>
<p>The <tt class="docutils literal">tests_full</tt> directory contains a test suite for running complete Scheme programs with Bob. To make sure Bob runs correctly, execute both <tt class="docutils literal">test_interpreter.py</tt> and <tt class="docutils literal">test_vm_compiler.py</tt>, and see that no errors are reported.</p>
<p>The file <tt class="docutils literal">testcases_utils.py</tt> contains the simple testing infrastructure code used to implement the actual tests, and the <tt class="docutils literal">testcases</tt> directory contains complete Scheme programs with expected output files. New test cases added to this directory will be automatically &quot;discovered&quot; and run by the testing scripts.</p>
</div>
</div>
<div class="section" id="barevm">
<h1>3&nbsp;&nbsp;&nbsp;BareVM</h1>
<p>BareVM is an implementation of the Bob VM in C++. It should work as a drop-in replacement for the Python Bob VM, with one small difference:</p>
<ul class="simple">
<li>Bob VM is implemented as a library of Python classes. The <tt class="docutils literal">examples/run_compiled.py</tt> example shows how to instantiate a <tt class="docutils literal">Deserializer</tt> object followed by a <tt class="docutils literal">BobVM</tt> object to execute VM bytecode from a <tt class="docutils literal">.bobc</tt> file.</li>
<li>BareVM compiles to a standalone executable that loads a <tt class="docutils literal">.bobc</tt> file, deserializes it and executes the bytecode in it.</li>
</ul>
<p>Why is BareVM interesting? Python is a powerful programming language - sometimes <em>too</em> powerful. In particular, when implementing a virtual machine, Python makes the task relatively easy. Its powerful object system with duck typing, reflection capabilities and built-in garbage collection is something most Python programmers take for granted.</p>
<p>And yet, most real-world VMs are implemented in C or C++, since a VM is one of those programs which are never fast enough. Compared to Python, these are low-level languages requiring much more of the implementation to be explicit. BareVM was created as an exercise in VM implementation in a low-level language. One of its most interesting features is a complete mark and sweep garbage collector. BareVM is also significantly faster than the Python Bob VM. Without any particular optimization efforts (following roughtly the same implementation logic as in the Python code), the performance is 5x  on many benchmarks.</p>
<div class="section" id="structure-of-the-barevm-sub-tree">
<h2>3.1&nbsp;&nbsp;&nbsp;Structure of the <tt class="docutils literal">barevm</tt> sub-tree</h2>
<ul class="simple">
<li><tt class="docutils literal">CMakeLists.txt</tt>: Main CMake definition file for Barevm</li>
<li><tt class="docutils literal">maketags.sh</tt>: Generate source code tags for Linux code editors</li>
<li><tt class="docutils literal">src/</tt>: BareVM source code</li>
<li><tt class="docutils literal">tests_unit/</tt>: BareVM unit tests, including a packaged version of <a class="reference external" href="http://code.google.com/p/googletest/">gtest</a>.</li>
</ul>
</div>
<div class="section" id="building">
<h2>3.2&nbsp;&nbsp;&nbsp;Building</h2>
<p>BareVM uses <a class="reference external" href="http://www.cmake.org">CMake</a> for its build:</p>
<ul class="simple">
<li>On Linux, run <tt class="docutils literal">cmake .</tt> in the BareVM <tt class="docutils literal">src</tt> directory, followed by <tt class="docutils literal">make</tt>. This creates two executables in the same directory: <tt class="docutils literal">barevm</tt> is the BareVM executable, <tt class="docutils literal">barevm_unittest</tt> runs the unit tests.</li>
<li>On Windows, run the <tt class="docutils literal">cmake</tt> GUI, configure the project and generate a Microsoft Visual Studio solution. Build the project from the solution.</li>
</ul>
</div>
<div class="section" id="running-and-testing">
<h2>3.3&nbsp;&nbsp;&nbsp;Running and testing</h2>
<p>The main BareVM driver (implemented in <tt class="docutils literal">src/main.cpp</tt>) is very simple - it expects a <tt class="docutils literal">.bobc</tt> file (containing serialized Bob bytecode) as a single argument, runs the bytecode and displays the output. It can be used as a drop-in replacement for <tt class="docutils literal">examples/run_compiled.py</tt>.</p>
<p>BareVM comes with a few simple unit tests which are run by executing <tt class="docutils literal">src/barevm_unittest</tt>, once this file is generated by the compiler.</p>
<p>The most comprehensive tests on BareVM are done by running the full tests. <tt class="docutils literal">tests_full/test_barevm.py</tt> uses the Python Bob compiler from Scheme to bytecode, in unison with BareVM to execute the tests, thus testing BareVM on the whole set of full testcases. By default, the path to barevm in <tt class="docutils literal">tests_full/test_barevm.py</tt> points to the executable generated on Linux. If you want to run these tests on Windows or move the executable to another location, modify the path accordingly.</p>
</div>
</div>
</div>
</body>
</html>
