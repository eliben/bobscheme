/******************************************************************************
** bob: Bytecode - code objects, instructions and deserialization.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include "mem.h"
#include "die.h"
#include "bytecode.h"
#include "cutils.h"


#define MAGIC_CONST         0x00010B0B

#define SER_TYPE_NULL       '0'
#define SER_TYPE_BOOLEAN    'b'
#define SER_TYPE_STRING     's'
#define SER_TYPE_SYMBOL     'S'
#define SER_TYPE_NUMBER     'n'
#define SER_TYPE_PAIR       'p'
#define SER_TYPE_INSTR      'i'
#define SER_TYPE_SEQUENCE   '['
#define SER_TYPE_CODEOBJECT 'c'


/* There are two kind of d_<name> functions here. 
** 1) d_<expect>_name: called before the type byte has been read from the 
**    stream. Before reading the object contents such functions first read
**    and match the type byte.
** 2) d_name: called after the type byte has been read and ascertained.
**    Such functions need only to read the object contents from the stream.
*/

/* Forward decls
*/
static BobObject* d_expect_object(FILE* stream);
static BobCodeObject* d_codeobject(FILE* stream);


static void d_match_type(FILE* stream, int type)
{
    int gottype = fgetc(stream);

    if (gottype != type)
        die("Deserialization type error: expected %c, got %c\n", type, gottype);
} 


static int d_word(FILE* stream)
{
    int word = fgetc(stream);
    word |= fgetc(stream) << 8;
    word |= fgetc(stream) << 16;
    word |= fgetc(stream) << 24;
    return word;
}


static BobObject* d_null(FILE* stream)
{
    return BobNull_new();
}


static BobObject* d_boolean(FILE* stream)
{
    int val = fgetc(stream);
    return BobBoolean_new(val == 1);
}


static BobObject* d_number(FILE* stream)
{
    int word = d_word(stream);
    return BobNumber_new(word);
}


static dstring d_string(FILE* stream)
{
    size_t len = d_word(stream);
    char* str = mem_alloc(len + 1);
    size_t i;

    for (i = 0; i < len; ++i) {
        str[i] = (char) fgetc(stream);
    }
    str[len] = 0;

    return dstring_from_cstr_and_len(str, len);
}


static BobObject* d_symbol(FILE* stream)
{
    dstring dstr = d_string(stream);
    BobObject* sym = BobSymbol_new(dstr);
    /* BobSymbol_new copies the string, so we no longer need this one.
    */
    dstring_free(dstr);
    return sym;
}


static BobObject* d_pair(FILE* stream)
{
    BobObject* first = d_expect_object(stream);
    BobObject* second = d_expect_object(stream);
    return BobPair_new(first, second);
}


static BobObject* d_expect_object(FILE* stream)
{
    int type = fgetc(stream);

    if (feof(stream))
        die("Deserialization error: stream ended prematurely\n");

    switch (type) {
        case SER_TYPE_NULL:
            return d_null(stream);
        case SER_TYPE_NUMBER:
            return d_number(stream);
        case SER_TYPE_BOOLEAN:
            return d_boolean(stream);
        case SER_TYPE_SYMBOL:
            return d_symbol(stream);
        case SER_TYPE_PAIR:
            return d_pair(stream);
        default:
            die("Deserialization error: expected object type, got %c\n", type);
    }

    assert(0);
    return 0;
}


static BobInstruction* d_expect_instruction(FILE* stream)
{
    BobInstruction* instr = mem_alloc(sizeof(*instr));
    unsigned word;
    d_match_type(stream, SER_TYPE_INSTR);
    word = d_word(stream);
    instr->opcode = word >> 24;
    instr->arg = word & 0xFFFFFF;
    return instr;
}


static BobBytecodeConstant* d_expect_bytecodeconstant(FILE* stream)
{
    BobBytecodeConstant* bytecodeconstant = mem_alloc(sizeof(*bytecodeconstant));
    int type = fgetc(stream);

    if (type == SER_TYPE_CODEOBJECT) {
        bytecodeconstant->type = CONSTANT_CODEOBJECT;
        bytecodeconstant->d.codeobj = d_codeobject(stream);
    }
    else {
        ungetc(type, stream);
        bytecodeconstant->type = CONSTANT_OBJECT;
        bytecodeconstant->d.obj = d_expect_object(stream);
    }

    return bytecodeconstant;
}


/* This macro should only be used inside d_codeobject to avoid code
** duplication.
*/
#define D_SEQUENCE(seqname, d_func)                     \
    do {d_match_type(stream, SER_TYPE_SEQUENCE);        \
        len = d_word(stream);                           \
        seqname = mem_alloc(len * sizeof(*(seqname)));  \
        for (i = 0; i < len; ++i)                       \
            seqname[i] = d_func(stream);                \
        } while (0);


static BobCodeObject* d_codeobject(FILE* stream)
{
    BobCodeObject* codeobject = mem_alloc(sizeof(*codeobject));
    unsigned len, i;

    d_match_type(stream, SER_TYPE_STRING);
    codeobject->name = d_string(stream);

    D_SEQUENCE(codeobject->args, d_string);
    codeobject->num_args = len;
    D_SEQUENCE(codeobject->constants, d_expect_bytecodeconstant);
    codeobject->num_constants = len;
    D_SEQUENCE(codeobject->varnames, d_string);
    codeobject->num_varnames = len;
    D_SEQUENCE(codeobject->code, d_expect_instruction);
    codeobject->codelen = len;

    return codeobject;
}


BobCodeObject* deserialize_bytecode(const char* filename)
{
    FILE* stream = fopen(filename, "rb");
    unsigned magic;
    if (!stream)
        die("Unable to open file for deserialization: %s\n", filename);
    
    magic = d_word(stream);
    if (magic != MAGIC_CONST)
        die("Deserialization error: magic constant mismatch: 0x%X\n", magic);

    d_match_type(stream, SER_TYPE_CODEOBJECT);
    return d_codeobject(stream);
}


void free_codeobject(BobCodeObject* codeobj)
{
    /* XXX: how to deallocate the bob objects?!
    ** also don't forget this function has to call itself recursively!!
    */
}
