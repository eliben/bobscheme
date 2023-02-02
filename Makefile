SHELL := bash

PYTHON := $(shell command -v python || command -v python3)
$(if $(PYTHON),,$(error No python or python3 binary found))

export PYTHONPATH := $(PWD)

PYTHON_TESTS := \
    test_interpreter \
    test_vm_compiler \
    test_barevm \

BAREVM_TESTS := \
    test_barevm \
    barevm_unittest \


default:

test: $(PYTHON_TESTS) barevm_unittest

$(BAREVM_TESTS):: barevm/barevm

$(PYTHON_TESTS)::
	$(PYTHON) tests_full/$@.py

barevm_unittest::
	$(MAKE) --no-print-directory -C barevm -f CMakeFiles/Makefile2 barevm_unittest

clean:
	git clean -dXf

barevm/barevm:
	$(if $(shell command -v cmake),,$(error No cmake binary found))
	cd barevm && cmake . && $(MAKE)
