//*****************************************************************************
// bob: Instructions and bytecode
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "bytecode.h"
#include "utils.h"

using namespace std;


static string opcode2str(unsigned opcode)
{
#define DEF_OP_STR(op)   case OP_##op: return #op
    switch (opcode) {
        DEF_OP_STR(CONST);
        DEF_OP_STR(LOADVAR);
        DEF_OP_STR(STOREVAR);
        DEF_OP_STR(DEFVAR);
        DEF_OP_STR(FUNCTION);
        DEF_OP_STR(POP);
        DEF_OP_STR(JUMP);
        DEF_OP_STR(FJUMP);
        DEF_OP_STR(RETURN);
        DEF_OP_STR(CALL);
        default: return "UNKNOWN";
    }
}


static string repr_nested(BobCodeObject* codeobj, unsigned nesting = 0)
{
    string prefix(nesting, ' ');
    string repr = format_string("%s----------\n%sCodeObject: %s\n", 
                    prefix.c_str(), prefix.c_str(), codeobj->name.c_str());
}


string BobCodeObject::repr() const
{
    return "";
}

