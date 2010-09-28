/******************************************************************************
** bob: The main VM implementation
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <stdio.h>
#include "mem.h"
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


struct BobVMState {
    stack valuestack;
    stack framestack;

    BobExecutionFrame* frame;

    FILE* output_stream;
};


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


static BobEnv* create_global_env()
{
    BobEnv* global_env = BobEnv_new(0);
#define ADD_BUILTIN(name)           \
    BobEnv_define_var(global_env,   \
            intern_cstring(#name),  \
            BobBuiltin_new(intern_cstring(#name), builtin_##name))
    ADD_BUILTIN(car);
    ADD_BUILTIN(cdr);
    ADD_BUILTIN(add);

    return global_env;
}


BobVMState* create_vm(FILE* output_stream)
{
    BobVMState* vm_state = mem_alloc(sizeof(*vm_state));
    vm_state->valuestack = stack_new(StackType_no_free);
    vm_state->framestack = stack_new(StackType_no_free);
    vm_state->frame = BobExecutionFrame_create(0, 0, 0);

    return vm_state;
}



