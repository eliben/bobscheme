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
    FILE* m_output_stream;

    // Stack for execution frames to implement procedure calls
    //
    stack<ExecutionFrame> m_framestack;

    // Stack for everything else
    //
    stack<BobObject*> m_valuestack;

    // The current execution frame
    //
    ExecutionFrame m_frame;

    //---------------------------------------------------------------

    BobEnvironment* create_global_env();

    BobObject* builtin_write(BuiltinArgs args);
    BobObject* builtin_debug_vm(BuiltinArgs args);
};


BobVM::BobVM(const string& output_file)
{
    d = new VMImpl;

    if (output_file == "") 
        d->m_output_stream = stdout;
    else {
        d->m_output_stream = fopen(output_file.c_str(), "w");

        if (!d->m_output_stream)
            throw VMError("Unable to open for output: " + output_file);
    }
}


BobVM::~BobVM()
{
    if (d->m_output_stream != stdout)
        fclose(d->m_output_stream);

    delete d;
}


// What follows is an attempt to create builtins that have access to the 
// internal state of the VM in a relatively clean way.
// The problem is that BuiltinProc is a pointer to a simple function that takes
// arguments and returns a value - it has no access to the VM. However, we
// also want to be able to create builtins that do have access to the VM.
// The way to do this is derive a special class from BobBuiltinProcedure.
// Instead of taking a BuiltinProc, BobVMBuiltinProcedure takes a VMImpl
// object and a pointer to a BobVM member function. 
// This way, by overriding exec(), it can be called in a way exactly
// similar to a normal BobBuiltinProcedure, but use its VM object reference
// and member function pointer to call a builtin function that's actually
// defined inside BobVM and thus has access to its state.
//
typedef BobObject* (VMImpl::*VMBuiltinProc)(BuiltinArgs args);

class BobVMBuiltinProcedure : public BobBuiltinProcedure
{
public:
    BobVMBuiltinProcedure(const string& name, VMImpl& vmobj, VMBuiltinProc builtin)
        : BobBuiltinProcedure(name, 0), m_vmobj(vmobj), m_builtin(builtin)
    {}

    virtual ~BobVMBuiltinProcedure();

    virtual BobObject* exec(BuiltinArgs args) const
    {
        // Invoke the member function via a pointer on the object
        //
        return (m_vmobj.*m_builtin)(args);
    }

private:
    VMImpl& m_vmobj;
    VMBuiltinProc m_builtin;
};


BobEnvironment* VMImpl::create_global_env()
{
    BuiltinsMap builtins_map = make_builtins_map();
    BobEnvironment* env = new BobEnvironment;

    // Add all the standard builtin procedures from the builtins module to the
    // environment
    //
    for (BuiltinsMap::const_iterator i = builtins_map.begin(); i != builtins_map.end(); ++i) {
        BobObject* proc = new BobBuiltinProcedure(i->first, i->second);
        env->define_var(i->first, proc);
    }

    // Now add the builtins defined as member functions of BobVM and have 
    // access to its state.
    //
    env->define_var("write", 
            new BobVMBuiltinProcedure("write", *this, &VMImpl::builtin_write));
    env->define_var("debug-vm", 
            new BobVMBuiltinProcedure("debug-vm", *this, &VMImpl::builtin_debug_vm));

    return env;
}


BobObject* VMImpl::builtin_write(BuiltinArgs args)
{
    string output_str;

    for (vector<BobObject*>::const_iterator i = args.begin(); i != args.end(); ++i) {
        output_str += (*i)->repr() + " ";
    }
    output_str += "\n";
    fputs(output_str.c_str(), m_output_stream);

    return 0;
}


BobObject* VMImpl::builtin_debug_vm(BuiltinArgs args)
{
    fputs("** debug called\n", m_output_stream);
    return 0;
}

