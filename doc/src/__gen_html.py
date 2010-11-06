import os, sys

FILES = [
    'bob_getting_started.txt',
    'bob_bytecode_reference.txt',
]

RST_EXE = 'rst2html' if sys.platform == 'win32' else 'rst2html.py'


for txtfile in FILES:
    filename, ext = os.path.splitext(txtfile)
    htmlfile = os.path.join('..', filename + '.html')
    cmd = '%s %s > %s' % (RST_EXE, txtfile, htmlfile)
    print 'Running:', cmd
    
    try:
        os.system(cmd)
    except Exception, e:
        print type(e), str(e)




