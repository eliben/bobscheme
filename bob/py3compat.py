#-------------------------------------------------------------------------------
# bob: py3compat.py
#
# Python 2/3 compatibility layer.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------

import sys
PY3 = sys.version_info[0] == 3


identity_func = lambda x: x

if PY3:
    input = input
    def bytes2str(b):
        return b.decode('utf-8')
else:
    input = raw_input
    bytes2str = identity_func
