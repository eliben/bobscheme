#-------------------------------------------------------------------------------
# bob: tests_full/testcases_utils.py
#
# Utilities for loading test cases from the testcases/ directory and running 
# them.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
import os, time, sys
from io import StringIO


class TestCase(object):
    def __init__(self, name, code, expected):
        self.name = name
        self.code = code
        self.expected = expected


def all_testcases(dir='testcases/'):
    for filename in sorted(os.listdir(dir)):
        if filename.endswith('.scm'):
            testname = os.path.splitext(filename)[0]
            fullpath = os.path.join(dir, filename)
            code = open(fullpath).read()
            
            exp_path = os.path.join(dir, testname + '.exp.txt')
            expected = open(exp_path).read()
            
            yield TestCase(testname, code, expected)


class StringIOUnicode(StringIO):
    """ For compatibility with both Python 2.6 and 3.x
    """
    def write(self, s):
        if sys.hexversion > 0x03000000:
            StringIO.write(self, s)
        else:
            StringIO.write(self, unicode(s))
        

def run_all_tests(runner, dir='testcases/'):
    """ Runs all tests from the directory with the given runner. A runner
        is a function accepting Scheme code as a string and an output stream
        for calls to 'write' - it's expected to run the code.
    """
    starttime = time.time()
    testcount = 1
    errorcount = 0

    for testcase in all_testcases():
        numdots = 25 - len(testcase.name)
        dots = '.' * numdots
        print('~~ Running test #%-3s [%s]%s.....' % (testcount, testcase.name, dots), end='')
        
        expected = testcase.expected.lstrip()
        ostream = StringIOUnicode()
        runner(testcase.code, ostream)
        
        if ostream.getvalue() == expected:
            print('OK')
        else:
            errorcount += 1
            print('ERROR')
            print('-------- Expected:\n%s' % expected)
            print('-------- Got:\n%s' % ostream.getvalue())
            print('-------- For code:\n%s' % testcase.code)

        testcount += 1

    if errorcount == 0:
        print('---- All tests ran OK ----')
    else:
        print('---- Tests had %s errors ----' % errorcount)
    print('Elapsed: %.4s sec' % (time.time() - starttime,))

