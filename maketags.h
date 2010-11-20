#!/bin/bash -v
ctags -R bob/
~/bin/pycscope.py -R
