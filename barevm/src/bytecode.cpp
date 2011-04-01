//*****************************************************************************
// bob: Instructions and bytecode
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "bytecode.h"
#include "utils.h"
#include <cassert>

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


static string repr_nested(const BobCodeObject* codeobj, unsigned nesting = 0)
{
    string prefix(nesting, ' ');

    // Header
    //
    string repr = format_string("%s----------\n%sCodeObject: %s\n", 
                    prefix.c_str(), prefix.c_str(), codeobj->name.c_str());

    // Arguments
    //
    repr.append(prefix + "Args: [");
    for (vector<string>::const_iterator arg = codeobj->args.begin(); arg != codeobj->args.end(); ++arg) {
        repr.append(*arg);
        repr.append(" ");
    }
    repr.append("]\n");

    // Code
    //
    for (size_t offset = 0; offset < codeobj->code.size(); ++offset) {
        BobInstruction instruction = codeobj->code[offset];
        repr.append(format_string("%s  %4u %-12s ", 
                        prefix.c_str(), 
                        offset, 
                        opcode2str(instruction.opcode).c_str()));

        string arg_repr;

        switch (instruction.opcode) {
            case OP_CONST: {
                arg_repr = format_string("%4d {= ", instruction.arg);
                const BobObject* constant = codeobj->constants[instruction.arg];
                arg_repr.append(constant->repr() + "}");
                break;
            }
            case OP_FUNCTION: {
                arg_repr = format_string("%4d {=\n", instruction.arg);
                const BobObject* constant = codeobj->constants[instruction.arg];
                const BobCodeObject* function = dynamic_cast<const BobCodeObject*>(constant);
                assert(function && "Expect BobCodeObject as argument of OP_FUNCTION");
                arg_repr.append(repr_nested(function, nesting + 8));
                break;
            }
            case OP_LOADVAR:
            case OP_STOREVAR:
            case OP_DEFVAR:
                arg_repr = format_string("%4d {=%s}", 
                                instruction.arg, 
                                codeobj->varnames[instruction.arg].c_str());
                break;
            case OP_FJUMP:
            case OP_JUMP:
            case OP_CALL:
                arg_repr = format_string("%4d", instruction.arg);
                break;
            case OP_POP:
            case OP_RETURN:
                break;
            default:
                assert(0 && "Invalid instruction");
        }

        repr.append(arg_repr + "\n");
    }

    return repr + prefix + "----------\n";
}


string BobCodeObject::repr() const
{
    return repr_nested(this, 0);
}


void BobCodeObject::gc_mark_pointed()
{
    for (vector<BobObject*>::iterator it = constants.begin(); it != constants.end(); ++it)
        (*it)->gc_mark();
}


