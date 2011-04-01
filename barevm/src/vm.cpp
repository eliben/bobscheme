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
#include "basicobjects.h"
#include <stack>
#include <deque>
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <typeinfo>

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

    string repr()
    {
        return format_string("Code: <%s> [PC=%d]", codeobject->name.c_str(), pc);
    }
};


// A closure is a code object (procedure) with an associated environment
// in which the closure was created.
//
class BobClosure : public BobObject
{
public:
    BobClosure(BobCodeObject* codeobject_, BobEnvironment* env_)
        : codeobject(codeobject_), env(env_)
    {}

    virtual ~BobClosure()
    {}

    virtual string repr() const
    {
        return format_string("<closure '%s'>", codeobject->name.c_str());
    }

    BobCodeObject* codeobject;
    BobEnvironment* env;

    virtual void gc_mark_pointed()
    {
        codeobject->gc_mark();
        env->gc_mark();
    }
};


struct VMImpl
{
    // The output stream for (write)
    //
    FILE* m_output_stream;

    // Explicit stack for execution frames to implement procedure calls.
    // deque<> is used for the stacks and not stack<> because stack<> doesn't
    // allow iteration over it, so it sucks. deque<> easily makes a stack
    // with push_back / pop_back / back.
    //
    deque<ExecutionFrame> m_framestack;

    // Stack for everything else
    //
    deque<BobObject*> m_valuestack;

    // The current execution frame
    //
    ExecutionFrame m_frame;

    size_t gc_size_threshold;

    //---------------------------------------------------------------

    // Builtins with access to VM state
    //
    BobObject* builtin_write(BuiltinArgs&);
    BobObject* builtin_debug_vm(BuiltinArgs&);
    BobObject* builtin_run_gc(BuiltinArgs&);
    BobObject* builtin_debug_gc(BuiltinArgs&);

    // Internal 
    //
    BobEnvironment* create_global_env();
    string repr_vm_state();

    void get_root_objects(vector<BobObject*>& roots);
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

    // Default GC size threshold
    //
    d->gc_size_threshold = 10 * 1024 * 1024;

    BobAllocator::get().register_vm_obj(this);
}


BobVM::~BobVM()
{
    if (d->m_output_stream != stdout)
        fclose(d->m_output_stream);
    delete d;
}


void BobVM::set_gc_size_threshold(size_t threshold)
{
    d->gc_size_threshold = threshold;
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

        // Let the GC run if required. 
        // Note: it's important to allow the GC to run only in-between
        // instructions, because during an instruction's execution, some
        // objects may not be reachable from the roots and the GC will
        // collect them if run. One example is builtin calls, where the
        // arguments are taken off the stack before passing control to
        // the builtin. If the builtin triggered a GC call, these arguments
        // would be collected which is a very bad thing. So, for extra 
        // safety, GC is not allowed to run arbitrarily.
        //
        BobAllocator::get().run_gc(d->gc_size_threshold);

        switch (instr.opcode) {
            case OP_CONST:
            {
                assert(instr.arg < cur_codeobj->constants.size() && "Constants offset in bounds");
                BobObject* val = cur_codeobj->constants[instr.arg];
                //cerr << "OP_CONST The repr of val on stack is " << val->repr() << endl;
                d->m_valuestack.push_back(val);
                break;
            }
            case OP_LOADVAR:
            {
                assert(instr.arg < cur_codeobj->varnames.size() && "Varnames offset in bounds");
                string varname = cur_codeobj->varnames[instr.arg];
                BobObject* val = d->m_frame.env->lookup_var(varname);
                if (!val)
                    throw VMError(format_string("Unknown variable '%s' referenced", varname.c_str()));
                d->m_valuestack.push_back(val);
                break;
            }
            case OP_STOREVAR:
            {
                assert(instr.arg < cur_codeobj->varnames.size() && "Varnames offset in bounds");
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobObject* val = d->m_valuestack.back();
                d->m_valuestack.pop_back();
                string varname = cur_codeobj->varnames[instr.arg];
                BobObject* retval = d->m_frame.env->set_var_value(varname, val);
                if (!retval)
                    throw VMError(format_string("Unknown variable '%s' referenced", varname.c_str()));
                break;
            }
            case OP_DEFVAR:
            {
                assert(instr.arg < cur_codeobj->varnames.size() && "Varnames offset in bounds");
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobObject* val = d->m_valuestack.back();
                d->m_valuestack.pop_back();
                d->m_frame.env->define_var(cur_codeobj->varnames[instr.arg], val);
                break;
            }
            case OP_POP:
            {
                // It's not a bug to generate instructions to pop the stack
                // when there's nothing to pop.
                //
                if (!d->m_valuestack.empty())
                    d->m_valuestack.pop_back();
                break;
            }
            case OP_JUMP:
            {
                d->m_frame.pc = instr.arg;
                break;
            }
            case OP_FJUMP:
            {
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobBoolean* bool_predicate = dynamic_cast<BobBoolean*>(d->m_valuestack.back());
                d->m_valuestack.pop_back();
                if (bool_predicate && !bool_predicate->value())
                    d->m_frame.pc = instr.arg;
                break;
            }
            case OP_FUNCTION:
            {
                assert(instr.arg < cur_codeobj->constants.size() && "Constants offset in bounds");
                BobObject* val = cur_codeobj->constants[instr.arg];
                BobCodeObject* func_codeobj = dynamic_cast<BobCodeObject*>(val);
                assert(val && "Expected code object as the argument to OP_FUNCTION");
                d->m_valuestack.push_back(new BobClosure(func_codeobj, d->m_frame.env));
                break;
            }
            case OP_RETURN:
            {
                assert(!d->m_framestack.empty() && "OP_RETURN needs non-empty frame stack");
                d->m_frame = d->m_framestack.back();
                d->m_framestack.pop_back();
                break;
            }
            case OP_CALL:
            {
                // For OP_CALL we have the function on top of the value stack,
                // followed by its arguments (in reverse order). The amount of
                // arguments is in the argument of the instruction.
                // The function is either a builtin procedure or a closure.
                //
                assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                BobObject* func_val = d->m_valuestack.back();
                d->m_valuestack.pop_back();
                vector<BobObject*> argvalues;

                // Take the function's arguments from the stack. The last
                // (right-most) argument is on top of the stack (first).
                //
                for (unsigned i = 0; i < instr.arg; ++i) {
                    assert(!d->m_valuestack.empty() && "Pop value from non-empty valuestack");
                    argvalues.push_back(d->m_valuestack.back());
                    d->m_valuestack.pop_back();
                }
                reverse(argvalues.begin(), argvalues.end());

                if (BobBuiltinProcedure* proc = dynamic_cast<BobBuiltinProcedure*>(func_val)) {
                    // Builtins wrap C++ procedures that should just be called
                    // with the arguments.
                    //
                    try {
                        BobObject* retval = proc->exec(argvalues);
                        d->m_valuestack.push_back(retval);
                    }
                    catch (const BuiltinError& err) {
                        throw VMError(err.what());
                    }
                }
                else if (BobClosure* closure = dynamic_cast<BobClosure*>(func_val)) {
                    // Extend the closure's environment with one where its code 
                    // object's arguments are bound to the values passed to it
                    // in the call.
                    //
                    if (argvalues.size() != closure->codeobject->args.size())
                        throw VMError(format_string("Calling procedure %s with %d args, expected %d",
                                        closure->codeobject->name.c_str(),
                                        argvalues.size(),
                                        closure->codeobject->args.size()));

                    BobEnvironment* call_env = new BobEnvironment(closure->env);
                    for (size_t i = 0; i < argvalues.size(); ++i) {
                        string argname = closure->codeobject->args[i];
                        BobObject* argvalue = argvalues[i];
                        call_env->define_var(argname, argvalue);
                    }

                    // To execute the procedure:
                    // 1. Save the current execution frame on the frame stack
                    // 2. Create a new frame from the closure's code object
                    //    and the extendend environment.
                    // 3. Start executing the frame by making it the current
                    //    frame with pc=0. The procedure's first instruction
                    //    will then execute in the next iteration of this 
                    //    loop.
                    //
                    d->m_framestack.push_back(d->m_frame);
                    ExecutionFrame new_frame;
                    new_frame.codeobject = closure->codeobject;
                    new_frame.pc = 0;
                    new_frame.env = call_env;
                    d->m_frame = new_frame;
                }
                else 
                    assert(0 && "Expected callable object on TOS for OP_CALL");

                break;
            }
            default:
                throw VMError(format_string("Invalid instruction opcode 0x%02X", instr.opcode));
        }
    }
}


void BobVM::gc_mark_roots()
{
    // current frame
    d->m_frame.codeobject->gc_mark();
    d->m_frame.env->gc_mark();

    // all objects in the value stack
    for (deque<BobObject*>::iterator it = d->m_valuestack.begin(); 
            it != d->m_valuestack.end(); ++it) {
        (*it)->gc_mark();
    }

    // all frames in the frame stack
    for (deque<ExecutionFrame>::iterator it = d->m_framestack.begin();
            it != d->m_framestack.end(); ++it) {
        it->codeobject->gc_mark();
        it->env->gc_mark();
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

    virtual ~BobVMBuiltinProcedure()
    {}

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
    env->define_var("__debug-vm", 
            new BobVMBuiltinProcedure("__debug-vm", *this, &VMImpl::builtin_debug_vm));
    env->define_var("__run-gc",
            new BobVMBuiltinProcedure("__run-gc", *this, &VMImpl::builtin_run_gc));
    env->define_var("__debug-gc",
            new BobVMBuiltinProcedure("__debug-gc", *this, &VMImpl::builtin_debug_gc));

    return env;
}


BobObject* VMImpl::builtin_write(BuiltinArgs& args)
{
    string output_str;

    for (BuiltinArgsIteratorConst i = args.begin(); i != args.end(); ++i) {
        output_str += (*i)->repr();

        if (i != args.end() - 1)
            output_str += " ";
    }
    output_str += "\n";
    fputs(output_str.c_str(), m_output_stream);

    return new BobNull;
}


BobObject* VMImpl::builtin_debug_vm(BuiltinArgs&)
{
    string str = repr_vm_state();
    fputs(str.c_str(), m_output_stream);
    return new BobNull;
}


static string frame_printer(ExecutionFrame frame)
{
    return "| " + frame.repr();
}


static string value_printer(BobObject* value)
{
    return "| " + value->repr();
}


template <class T>
static string repr_stack(deque<T>& thestack, string name, string (*printer)(T))
{
    string head = string(8 + name.size(), '-');
    string str = format_string("+%s+\n| %s stack |\n+%s+\n\n",
                    head.c_str(), name.c_str(), head.c_str());

    bool tos = true;
    for (typename deque<T>::reverse_iterator rit = thestack.rbegin();
            rit != thestack.rend(); ++rit) {
        str += "     |--------\n";
        str += tos ? "TOS: " : "     ";
        str += printer(*rit) + "\n";

        tos = false;
    }
    str += "     |--------\n";

    return str;
}


string VMImpl::repr_vm_state()
{
    string str = repr_stack(m_valuestack, "Value", value_printer);
    str += "\n" + repr_stack(m_framestack, "Frame", frame_printer);
    return str;
}


BobObject* VMImpl::builtin_run_gc(BuiltinArgs&)
{
    // Force a GC run by setting threshold to 0
    BobAllocator::get().run_gc(0);
    return new BobNull;
}


// Print debugging information about the allocator/garbage collector.
// If an argument is given and it's #t, print all live objects.
//
BobObject* VMImpl::builtin_debug_gc(BuiltinArgs& args)
{
    string str = BobAllocator::get().stats_general();
    fputs(str.c_str(), m_output_stream);

    if (args.size() > 0) {
        if (BobBoolean* boolean = dynamic_cast<BobBoolean*>(args[0])) {
            if (boolean->value() == true) {
                str = BobAllocator::get().stats_all_live();
                fputs(str.c_str(), m_output_stream);
            }
        }
    }
    return new BobNull;
}

