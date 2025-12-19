//*****************************************************************************
// bob: Instructions and bytecode
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BYTECODE_H
#define BYTECODE_H

#include "bobobject.h"
#include <string>
#include <vector>


// VM instruction opcodes
//
const unsigned OP_CONST      = 0x00;
const unsigned OP_LOADVAR    = 0x10;
const unsigned OP_STOREVAR   = 0x11;
const unsigned OP_DEFVAR     = 0x12;
const unsigned OP_FUNCTION   = 0x20;
const unsigned OP_POP        = 0x30;
const unsigned OP_JUMP       = 0x40;
const unsigned OP_FJUMP      = 0x41;
const unsigned OP_RETURN     = 0x50;
const unsigned OP_CALL       = 0x51;
const unsigned OP_INVALID    = 0xFF;


// An instruction is a POD type containing the opcode and a single
// numeric argument (for instructions that need it)
//
struct BobInstruction 
{
    BobInstruction(unsigned opcode_ = OP_INVALID, unsigned arg_ = 0)
        : opcode(opcode_), arg(arg_)
    {}

    unsigned opcode;
    unsigned arg;
};


class BobCodeObject : public BobObject
{
public:
    BobCodeObject()
    {}

    virtual ~BobCodeObject()
    {}

    std::string repr() const;

    std::string name;
    std::vector<std::string> args;
    std::vector<std::string> varnames;
    std::vector<BobObject*> constants;
    std::vector<BobInstruction> code;

    virtual void gc_mark_pointed();
};

#endif /* BYTECODE_H */

