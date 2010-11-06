#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "bytecode.h"
#include "dstring.h"
#include "die.h"
#include "intern.h"
#include "environment.h"
#include "vm.h"


void zenv()
{
    BobEnv* env = BobEnv_new(NULL);
    BobEnv* envc = BobEnv_new(env);
    
    dstring djoe = intern_dstring(dstring_new("joe"));
    dstring dmoby = intern_dstring(dstring_new("moby"));
    dstring dliljoe = intern_dstring(dstring_new("liljoe"));

    BobEnv_define_var(env, djoe, (BobObject*) 50);
    BobEnv_define_var(env, dmoby, (BobObject*) 60);

    BobEnv_define_var(envc, dmoby, (BobObject*) 10);
    BobEnv_define_var(envc, dliljoe, (BobObject*) 312);

    printf("joe in env: %u\n", (size_t)BobEnv_lookup(env, djoe));
    printf("joe in envc: %u\n", (size_t)BobEnv_lookup(envc, djoe));
    printf("liljoe in env: %u\n", (size_t)BobEnv_lookup(env, dliljoe));
    printf("liljoe in envc: %u\n", (size_t)BobEnv_lookup(envc, dliljoe));
    printf("moby in env: %u\n", (size_t)BobEnv_lookup(env, dmoby));
    printf("moby in envc: %u\n", (size_t)BobEnv_lookup(envc, dmoby));

    BobEnv_set_var_val(envc, dmoby, (BobObject*)11114);
    BobEnv_set_var_val(envc, djoe, (BobObject*)11434);

    printf("joe in env: %u\n", (size_t)BobEnv_lookup(env, djoe));
    printf("joe in envc: %u\n", (size_t)BobEnv_lookup(envc, djoe));
    printf("liljoe in env: %u\n", (size_t)BobEnv_lookup(env, dliljoe));
    printf("liljoe in envc: %u\n", (size_t)BobEnv_lookup(envc, dliljoe));
    printf("moby in env: %u\n", (size_t)BobEnv_lookup(env, dmoby));
    printf("moby in envc: %u\n", (size_t)BobEnv_lookup(envc, dmoby));
}

int main(int argc, const char* argv[])
{
    BobCodeObject* codeobject;
    /*dstring ds = dstring_new("blahamuha");*/
    /*BobObject* bs = BobSymbol_new(ds);*/
    /*exit(1);*/

    if (argc != 2)
        die("Call: %s <filename>\n", argv[0]);

    if (argv[1][0] == 'e') {
        zenv();
        exit(1);
    }

    codeobject = deserialize_bytecode(argv[1]);
    {
        dstring repr = dstring_empty();
        BobCodeObject_repr(codeobject, repr);
        puts(dstring_cstr(repr));
    }
    {
        init_vm(stderr);
        vm_run_code(codeobject);
    }

    return 0;
}
