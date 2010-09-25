#include <stdio.h>
#include "object.h"
#include "bytecode.h"


int main(int argc, const char* argv[])
{
    BobObject* bo = BobNumber_new(1553);
    printf("%d\n", bo->d.num);
    return 0;
}
