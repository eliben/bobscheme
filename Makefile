SHELL := bash

PYTHON := $(shell command -v python || command -v python3)
$(if $(PYTHON),,$(error No python or python3 binary found))

export PYTHONPATH := $(PWD)

PYTHON_TESTS := \
    test_interpreter \
    test_vm_compiler \

BAREVM_TESTS := \
    test_barevm \
    barevm_unittest \

default:

test: test-python

test-all: test-python test-barevm

test-python: $(PYTHON_TESTS)

test-barevm: $(BAREVM_TESTS)

clean:
	git clean -dXf

$(PYTHON_TESTS)::
	$(PYTHON) tests_full/$@.py

$(BAREVM_TESTS):: barevm/barevm

test_barevm:: barevm/barevm
	$(PYTHON) tests_full/$@.py

barevm_unittest::
	$(MAKE) --no-print-directory -C barevm -f CMakeFiles/Makefile2 barevm_unittest
	./barevm/barevm_unittest

barevm/barevm: barevm/src/*.cpp barevm/src/*.h
	$(if $(shell command -v cmake),,$(error No cmake binary found))
	cd barevm && cmake . && $(MAKE)
