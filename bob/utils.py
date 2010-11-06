#-------------------------------------------------------------------------------
# utils.py
#
# Some generic utilities
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
from collections import deque
import struct
import time


def flatten(iterables):
    """ Flatten an iterable of iterables. Returns a generator.
        
        list(flatten([[2, 3], [5, 6]])) => [2, 3, 5, 6]
    """
    return (elem for iterable in iterables for elem in iterable)


def list_find_or_append(lst, item):
    """ If item is in the list, return its index. Otherwise, append it to the 
        list and return its index.
        
        Note: may modify the list
    """
    try:
        return lst.index(item)
    except ValueError:
        lst.append(item)
        return len(lst) - 1


def pack_word(word, big_endian=False):
    """ Packs a 32-bit word into a binary data string.
    """
    endian = ">" if big_endian else "<"
    return struct.pack("%sL" % endian, word)
    

def unpack_word(str, big_endian=False):
    """ Unpacks a 32-bit word from binary data.
    """
    endian = ">" if big_endian else "<"
    return struct.unpack("%sL" % endian, str)[0]

def byte_literal(b):
    """ If b is already a byte literal, return it. Otherwise, b is 
        an integer which should be converted to a byte literal.

        This function is for compatibility with Python 2.6 and 3.x
    """
    if isinstance(b, int):
        return bytes([b])
    else:
        return b

def get_bytes_from_iterator(it, nbytes):
    """ Takes a few bytes from an iterator and returns them as a single
        bytes object.
    """
    lst = [next(it) for n in range(nbytes)]
    return b''.join(byte_literal(b) for b in lst)


class Stack(object):
    """ A simple stack abstraction.
    """
    def __init__(self):
        self.d = deque()
        
    def push(self, item):
        self.d.append(item)
    
    def pop(self):
        """ Pop and return element at top-of-stack.
            Raise IndexError if the stack is empty.
        """
        return self.d.pop()
    
    def peek(self, index=-1):
        """ Peek at some element of the stack without removing it. By default
            the element at the top-of-stack is returned. Pass a negative 
            index similar to Python list syntax for arbitrary peeking.
            
            Raise IndexError for invalid indices.
        """
        return self.d[index]

    def __len__(self):
        return len(self.d)


class Timer(object):
    def __init__(self, name=None):
        self.name = name
    
    def __enter__(self):
        self.tstart = time.time()
        
    def __exit__(self, type, value, traceback):
        if self.name:
            print('[%s]' % self.name)
        print('Elapsed: %s' % (time.time() - self.tstart))

