//*****************************************************************************
// bob: The virtual machine implementation
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "vm.h"
#include "bytecode.h"
#include "environment.h"
#include "builtins.h"
#include <stack>
#include <cstdio>
#include <stack>

using namespace std;


// Encapsulates the VM state - "execution frame". The frame consists of the
// current code object being executed, the pc (program counter) offset into
// it to know which instruction is next to execute, and the current
// environment in which the code object is being executed.
//
struct ExecutionFrame
{
    BobCodeObject* codeobject;
    unsigned pc;
    BobEnvironment* env;
};


struct VMImpl
{
    // The output stream for (write)
    //
    FILE* output_stream;

    // Stack for execution frames to implement procedure calls
    //
    stack<ExecutionFrame> framestack;

    // Stack for everything else
    //
    stack<BobObject*> valuestack;

    // The current execution frame
    //
    ExecutionFrame frame;

    //---------------------------------------------------------------

    BobEnvironment* create_global_env();
};


BobVM::BobVM(const string& output_file)
{
    d = new VMImpl;

    if (output_file == "") 
        d->output_stream = stdout;
    else {
        d->output_stream = fopen(output_file.c_str(), "w");

        if (!d->output_stream)
            throw VMError("Unable to open for output: " + output_file);
    }
}


BobVM::~BobVM()
{
    if (d->output_stream != stdout)
        fclose(d->output_stream);

    delete d;
}


BobEnvironment* VMImpl::create_global_env()
{
    BobEnvironment* env = new BobEnvironment;
    return 0;
}

