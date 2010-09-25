#include <stdio.h>
#include "object.h"
#include "bytecode.h"
#include "die.h"


int main(int argc, const char* argv[])
{
    BobCodeObject* codeobject;

    if (argc != 2)
        die("Call: %s <filename>\n", argv[0]);

    codeobject = deserialize_bytecode(argv[1]);

    return 0;
}
