//*****************************************************************************
// bob: Bytecode objects and deserialization
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BYTECODE_H
#define BYTECODE_H

#include "bobobject.h"
#include <string>
#include <vector>
#include <stdexcept>


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


// An Instruction is a POD type containing the opcode and a single
// numeric argument (for instructions that need it)
//
struct Instruction 
{
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
    std::vector<const BobObject*> constants;
    std::vector<Instruction> code;
};


// The exception type thrown by the deserializer
//
struct DeserializationError : public std::runtime_error
{
    DeserializationError(const std::string& reason) 
        : std::runtime_error(reason)
    {}
};

// Given a bytecode file, deserializes it into a new BobCodeObject
//
BobCodeObject* deserialize_bytecode(std::string filename); 

#endif /* BYTECODE_H */

