#!/bin/bash
ctags -R src/ include/ utils/
cscope -R -b

