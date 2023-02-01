SHELL := bash

PYTHON := $(shell command -v python || command -v python3)
$(if $(PYTHON),,$(error No python or python3 binary found))

export PYTHONPATH := $(PWD)

TESTS := \
    test_interpreter \
    test_vm_compiler \
    test_barevm \


default:

test: $(TESTS)

test_barevm:: barevm/barevm

$(TESTS)::
	$(PYTHON) tests_full/$@.py

clean:
	git status --ignored | \
	    grep -A9999 '^Ignored' | \
	    grep $$'^\t' | \
	    cut -f2 | \
	    xargs -r $(RM) -r

barevm/barevm:
	$(if $(shell command -v cmake),,$(error No cmake binary found))
	cd barevm && cmake . && $(MAKE)
