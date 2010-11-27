//*****************************************************************************
// bob: The virtual machine implementation
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "vm.h"
#include "utils.h"
#include "bytecode.h"
#include "environment.h"
#include "builtins.h"
#include <stack>
#include <cstdio>
#include <stack>
#include <cassert>

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

    BobObject* builtin_write(BuiltinArgs& args);
    BobObject* builtin_debug_vm(BuiltinArgs& args);
};


BobVM::BobVM(const string& output_file)
    : d(new VMImpl)
{
    if (output_file == "") 
        d->m_output_stream = stdout;
    else {
        d->m_output_stream = fopen(output_file.c_str(), "w");

        if (!d->m_output_stream)
            throw VMError("Unable to open for output: " + output_file);
    }

    d->m_frame.codeobject = 0;
    d->m_frame.pc = 0;
    d->m_frame.env = d->create_global_env();
}


BobVM::~BobVM()
{
    if (d->m_output_stream != stdout)
        fclose(d->m_output_stream);
}


void BobVM::run(BobCodeObject* codeobj)
{
    if (!codeobj)
        return;

    d->m_frame.codeobject = codeobj;
    d->m_frame.pc = 0;

    // The big VM loop!
    //
    while (true) {
        BobCodeObject* cur_codeobj = d->m_frame.codeobject;

        // Get the next instruction from the current code object. If there's
        // no more instructions, this must be a top-level codeobject, in
        // which case the program is done.
        //
        BobInstruction instr;
        if (d->m_frame.pc >= cur_codeobj->code.size()) {
            if (d->m_framestack.size() == 0) 
                return;
            else
                throw VMError("Code object ended prematurely");
        }
        else {
            instr = cur_codeobj->code[d->m_frame.pc];
            d->m_frame.pc++;
        }

        switch (instr.opcode) {
            case OP_CONST:
            {
                assert(instr.arg < cur_codeobj->constants.size() && "Constants offset in bounds");
                BobObject* val = cur_codeobj->constants[instr.arg];
                d->m_valuestack.push(val);
                break;
            }
            case OP_LOADVAR:
            {
                assert(instr.arg < cur_codeobj->varnames[instr.arg].size() && "Varnames offset in bounds");
                string varname = cur_codeobj->varnames[instr.arg];
                BobObject* val = d->m_frame.env->lookup_var(varname);
                d->m_valuestack.push(val);
                break;
            }
            case OP_STOREVAR:
            {
                assert(instr.arg < cur_codeobj->varnames[instr.arg].size() && "Varnames offset in bounds");
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobObject* val = d->m_valuestack.top();
                d->m_valuestack.pop();
                d->m_frame.env->set_var_value(cur_codeobj->varnames[instr.arg], val);
                break;
            }
            case OP_DEFVAR:
            {
                assert(instr.arg < cur_codeobj->varnames[instr.arg].size() && "Varnames offset in bounds");
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobObject* val = d->m_valuestack.top();
                d->m_valuestack.pop();
                d->m_frame.env->define_var(cur_codeobj->varnames[instr.arg], val);
                break;
            }
            case OP_POP:
            {
                // It's not a bug to generate instructions to pop the stack
                // when there's nothing to pop.
                //
                if (!d->m_valuestack.empty())
                    d->m_valuestack.pop();
            }
            default:
                throw VMError(format_string("Invalid instruction opcode 0x%02X", instr.opcode));
        }
    }
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
typedef BobObject* (VMImpl::*VMBuiltinProc)(BuiltinArgs& args);

class BobVMBuiltinProcedure : public BobBuiltinProcedure
{
public:
    BobVMBuiltinProcedure(const string& name, VMImpl& vmobj, VMBuiltinProc builtin)
        : BobBuiltinProcedure(name, 0), m_vmobj(vmobj), m_builtin(builtin)
    {}

    virtual ~BobVMBuiltinProcedure();

    virtual BobObject* exec(BuiltinArgs& args) const
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


BobObject* VMImpl::builtin_write(BuiltinArgs& args)
{
    string output_str;

    for (BuiltinArgsIteratorConst i = args.begin(); i != args.end(); ++i) {
        output_str += (*i)->repr() + " ";
    }
    output_str += "\n";
    fputs(output_str.c_str(), m_output_stream);

    return 0;
}


BobObject* VMImpl::builtin_debug_vm(BuiltinArgs& args)
{
    (void)args;
    fputs("** debug called\n", m_output_stream);
    return 0;
}

