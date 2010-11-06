/******************************************************************************
** bob: The main VM implementation
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <stdio.h>
#include "mem.h"
#include "die.h"
#include "stack.h"
#include "intern.h"
#include "object.h"
#include "environment.h"
#include "bytecode.h"
#include "builtins.h"
#include "vm.h"


typedef struct BobExecutionFrame {
    BobCodeObject* codeobj;
    size_t pc;
    BobEnv* env;
} BobExecutionFrame;


typedef struct BobVMState {
    stack valuestack;
    stack framestack;

    BobExecutionFrame* frame;

    BobObject* output_port;
} BobVMState;

/* Current VM state.
*/
static BobVMState* vm_state;


static BobExecutionFrame* BobExecutionFrame_create(BobCodeObject* codeobj, size_t pc, BobEnv* env)
{
    BobExecutionFrame* frame = mem_alloc(sizeof(*frame));
    frame->codeobj = codeobj;
    frame->pc = pc;
    frame->env = env;
    return frame;
}


static void BobExecutionFrame_free(BobExecutionFrame* frame)
{
    mem_free(frame);
}


/* Wraps the calls to builtin_write, ensuring that the current output port
** is given to it when no port was supplied.
** I.e. calls to (write obj) will be translated to (write obj <cur-port>)
*/
static BobObject* builtin_write_wrapper(size_t nargs, struct BobObject** args)
{
    BobObject* retval;

    if (nargs == 1) {
        BobObject** newargs = mem_alloc(2 * sizeof(*newargs));
        newargs[0] = args[0];
        newargs[1] = vm_state->output_port;

        retval = builtin_write(2, newargs);
        mem_free(newargs);
    }
    else {
        retval = builtin_write(nargs, args);
    }
    return retval;
}


static BobEnv* create_global_env()
{
    BobEnv* global_env = BobEnv_new(NULL);
#define ADD_BUILTIN(name)           \
    BobEnv_define_var(global_env,   \
            intern_cstring(#name),  \
            BobBuiltin_new(intern_cstring(#name), builtin_##name))
    ADD_BUILTIN(car);
    ADD_BUILTIN(cdr);
    ADD_BUILTIN(add);

    BobEnv_define_var(global_env, 
            intern_cstring("write"), 
            BobBuiltin_new(intern_cstring("write"), builtin_write_wrapper));

    return global_env;
}


static BobInstruction* get_next_instruction()
{
    size_t cur_pc = vm_state->frame->pc; 

    if (cur_pc >= vm_state->frame->codeobj->codelen)
        return NULL;
    else {
        BobInstruction* instr = vm_state->frame->codeobj->code[cur_pc];
        vm_state->frame->pc++;
        return instr;
    }
}


/* Create a new, empty VM state object.
*/
static BobVMState* create_new_vm_state(FILE* output_stream)
{
    BobVMState* vm_state = mem_alloc(sizeof(*vm_state));
    vm_state->valuestack = stack_new(StackType_no_free);
    vm_state->framestack = stack_new(StackType_no_free);
    vm_state->frame = BobExecutionFrame_create(NULL, 0, NULL);
    vm_state->output_port = BobPort_new(output_stream);

    return vm_state;
}


void init_vm(FILE* output_stream)
{
    vm_state = create_new_vm_state(output_stream);
    vm_state->frame->env = create_global_env();
}


void vm_run_code(struct BobCodeObject* codeobj)
{
    vm_state->frame->pc = 0;
    vm_state->frame->codeobj = codeobj;

    while (TRUE) {
        BobInstruction* instr = get_next_instruction();

        /* If there are no more instructions in this code object, we've either
        ** finished the top-level code object, or it's an error (all but the
        ** top-level code objects must not run out of instructions (they 
        ** return with OP_RETURN).
        */
        if (!instr) {
            if (stack_top(vm_state->framestack) == NULL) 
                break;
            else
                die("VM Error: Code object ended prematurely\n");
        }

        switch (instr->opcode) {
            case OP_CONST:

        }

        printf("PC: %u, instr 0x%0X, arg 0x%0X\n", vm_state->frame->pc, instr->opcode, instr->arg);
    }
}

