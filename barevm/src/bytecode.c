/******************************************************************************
** bob: Bytecode - code objects, instructions and deserialization.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <stdio.h>
#include <string.h>
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
    size_t nread = fread(str, 1, len, stream);
    if (nread != len)
        die("Deserialization error: string ended earlier than expected (%u bytes read instead of %u)\n", nread, len);
    str[len] = '\0';

    return dstring_from_cstr_and_len(str, len);
}


static dstring d_expect_string(FILE* stream)
{
    d_match_type(stream, SER_TYPE_STRING);
    return d_string(stream);
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

    codeobject->name = d_expect_string(stream);

    D_SEQUENCE(codeobject->args, d_expect_string);
    codeobject->num_args = len;
    D_SEQUENCE(codeobject->constants, d_expect_bytecodeconstant);
    codeobject->num_constants = len;
    D_SEQUENCE(codeobject->varnames, d_expect_string);
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


static dstring opcode2str(unsigned opcode)
{
#define DEF_OP_STR(op)   case OP_##op: return dstring_new(#op)
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
        default: return dstring_new("UNKNOWN");
    }
}

static void BobCodeObject_repr_nested(BobCodeObject* codeobj, dstring repr, size_t nesting)
{
    size_t i;

    /* prefix: blank space before each line in the representation to nicely
    ** show the nesting of code objects.
    */
    char* prefix = mem_alloc(nesting + 1);
    memset(prefix, ' ', nesting);
    prefix[nesting] = '\0';

    /* Header
    */
    dstring_concat_cstr(repr, prefix);
    dstring_concat_cstr(repr, "----------\n");
    dstring_concat_cstr(repr, prefix);
    dstring_concat_cstr(repr, "CodeObject: ");
    dstring_concat(repr, codeobj->name);
    dstring_concat_cstr(repr, "\n");

    /* Args
    */
    dstring_concat_cstr(repr, prefix);
    dstring_concat_cstr(repr, "Args: [");

    for (i = 0; i < codeobj->num_args; ++i) {
        dstring_concat(repr, codeobj->args[i]);
        dstring_concat_cstr(repr, " ");
    }
    dstring_concat_cstr(repr, "]\n");

    /* Code
    */
    for (i = 0; i < codeobj->codelen; ++i) {
        BobInstruction* instr = codeobj->code[i];
        dstring opcodestr = opcode2str(instr->opcode);
        dstring op_format = dstring_format("  %4u %-12s ", i, dstring_cstr(opcodestr));
        dstring arg_format = 0;

        dstring_concat_cstr(repr, prefix);
        dstring_concat(repr, op_format);

        switch (instr->opcode) {
            case OP_CONST:
                arg_format = dstring_format("%4d {= ", instr->arg);
                assert(codeobj->constants[instr->arg]->type == CONSTANT_OBJECT);
                BobObject_repr(codeobj->constants[instr->arg]->d.obj, arg_format);
                dstring_concat_cstr(arg_format, "}");
                break;
            case OP_LOADVAR:
            case OP_STOREVAR:
            case OP_DEFVAR:
                arg_format = dstring_format("%4d {= ", instr->arg);
                dstring_concat(arg_format, codeobj->varnames[instr->arg]);
                dstring_concat_cstr(arg_format, "}");
                break;
            case OP_FJUMP:
            case OP_JUMP:
            case OP_CALL:
                arg_format = dstring_format("%4d", instr->arg);
                break;
            case OP_FUNCTION:
                arg_format = dstring_format("%4d {=\n", instr->arg);
                assert(codeobj->constants[instr->arg]->type == CONSTANT_CODEOBJECT);
                BobCodeObject_repr_nested(codeobj->constants[instr->arg]->d.codeobj, arg_format, nesting + 8);
                break;
            case OP_POP:
            case OP_RETURN:
                arg_format = dstring_empty();
                break;
            default:
                assert(0 && "unknown instruction opcode");
        }
        dstring_concat(repr, arg_format);
        dstring_concat_cstr(repr, "\n");

        dstring_free(opcodestr);
        dstring_free(arg_format);
        dstring_free(op_format);
    }

    dstring_concat_cstr(repr, prefix);
    dstring_concat_cstr(repr, "----------\n");
    mem_free(prefix);
}


void BobCodeObject_repr(BobCodeObject* codeobj, dstring repr)
{
    BobCodeObject_repr_nested(codeobj, repr, 0);
}


void BobCodeObject_free(BobCodeObject* codeobj)
{
    /* XXX: how to deallocate the bob objects?!
    ** also don't forget this function has to call itself recursively!!
    */
}
