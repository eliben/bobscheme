/******************************************************************************
** bob: Bytecode - code objects, instructions and deserialization.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef BYTECODE_H
#define BYTECODE_H

#include "dstring.h"
#include "object.h"


/* Bob bytecode op-codes.
*/
#define OP_CONST    0x00
#define OP_LOADVAR  0x10
#define OP_STOREVAR 0x11
#define OP_DEFVAR   0x12
#define OP_FUNCTION 0x20
#define OP_POP      0x30
#define OP_JUMP     0x40
#define OP_FJUMP    0x41
#define OP_RETURN   0x50
#define OP_CALL     0x51


/* Bytecode instruction executable by the VM.
*/
typedef struct BobInstruction {
    unsigned opcode;
    unsigned arg;
} BobInstruction;


typedef enum {
    CONSTANT_OBJECT, CONSTANT_CODEOBJECT
} BobBytecodeConstantType;


/* Bytecode constants can be either regular objects or code objects.
*/
struct BobCodeObject;

typedef struct BobBytecodeConstant {
    BobBytecodeConstantType type;

    union {
        BobObject* obj;
        struct BobCodeObject* codeobj;
    } d;
} BobBytecodeConstant;


/* Code object - a Scheme procedure in its compiled and assembled form,
** suitable for execution by the VM.
*/
typedef struct BobCodeObject {
    dstring name;
    unsigned num_args;
    dstring* args;
    unsigned num_varnames;
    dstring* varnames;
    unsigned num_constants;
    BobBytecodeConstant** constants;
    unsigned codelen;
    BobInstruction** code;
} BobCodeObject;


BobCodeObject* deserialize_bytecode(const char* filename);
void BobCodeObject_repr(BobCodeObject* codeobj, dstring repr);
void BobCodeObject_free(BobCodeObject* codeobj);


#endif /* BYTECODE_H */
