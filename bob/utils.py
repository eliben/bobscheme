#-------------------------------------------------------------------------------
# utils.py
#
# Some generic utilities
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from collections import deque
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
            print '[%s]' % self.name,
        print 'Elapsed: %s' % (time.time() - self.tstart)

