#include <assert.h>
#include <stdio.h>
#include "object.h"
#include "bytecode.h"
#include "dstring.h"
#include "die.h"


int main(int argc, const char* argv[])
{
    /*BobCodeObject* codeobject;*/

    /*if (argc != 2)*/
        /*die("Call: %s <filename>\n", argv[0]);*/

    /*codeobject = deserialize_bytecode(argv[1]);*/
    /*{*/
        /*dstring hoe = dstring_new("armageddon");*/
        /*dstring_concat_cstr(hoe, " is done");*/
        /*printf("%s\n", dstring_cstr(hoe));*/
    /*}*/
    {
        BobObject* ss = BobSymbol_new(dstring_new("kaclso"));
        BobObject* num = BobNumber_new(3382);
        BobObject* p1 = BobPair_new(ss, num);
        BobObject* p2 = BobPair_new(ss, BobNull_new());
        BobObject* p3 = BobPair_new(p1, p2);
        BobObject* p4 = BobPair_new(p2, p3);

        dstring repr = dstring_empty();

        BobObject_repr(p4, repr);

        printf("repr: %s\n", dstring_cstr(repr));
    }

    return 0;
}
