SHELL := bash

PYTHON := $(shell command -v python || command -v python3)
$(if $(PYTHON),,$(error No python or python3 binary found))

export PYTHONPATH := $(PWD)

PYTHON_TESTS := \
    test_interpreter \
    test_vm_compiler \

test-all: test-python test-barevm

test-python: $(PYTHON_TESTS)

test-barevm: barevm/barevm
	$(PYTHON) tests_full/test_barevm.py

clean:
	git clean -dXf

$(PYTHON_TESTS)::
	$(PYTHON) tests_full/$@.py

barevm/barevm: barevm/*.cpp barevm/*.h
	cd barevm && $(MAKE)
