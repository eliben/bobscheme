//*****************************************************************************
// bob: (De)serialization of Bob code objects
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "bytecode.h"
#include "serialization.h"
#include "basicobjects.h"
#include "utils.h"
#include <cstdio>
#include <cassert>

using namespace std;


const unsigned MAGIC_CONST = 0x00010B0B;

const unsigned char SER_TYPE_NULL        = '0';
const unsigned char SER_TYPE_BOOLEAN     = 'b';
const unsigned char SER_TYPE_STRING      = 's';
const unsigned char SER_TYPE_SYMBOL      = 'S';
const unsigned char SER_TYPE_NUMBER      = 'n';
const unsigned char SER_TYPE_PAIR        = 'p';
const unsigned char SER_TYPE_INSTR       = 'i';
const unsigned char SER_TYPE_SEQUENCE    = '[';
const unsigned char SER_TYPE_CODEOBJECT  = 'c';


class BytecodeStream 
{
public:
    BytecodeStream(const char* filename)
    {
        stream = fopen(filename, "rb");
        if (!stream)
            throw DeserializationError("Unable to open file for deserialization");
    }

    unsigned char read_byte()
    {
        int b = fgetc(stream);
        if (b == EOF)
            throw DeserializationError("Stream ended prematurely");
        return b & 0xFF;
    }

    unsigned read_word()
    {
        // Little endian
        //
        unsigned word = read_byte();
        word |= read_byte() << 8;
        word |= read_byte() << 16;
        word |= read_byte() << 24;
        return word;
    }

    string read_string(unsigned len)
    {
        char* buf = new char[len + 1];
        unsigned nread = fread(buf, 1, len, stream);
        if (nread != len)
            throw DeserializationError("Stream ended prematurely");
        buf[nread] = '\0';
        return buf;
    }

    ~BytecodeStream()
    {
        fclose(stream);
    }
private:
    FILE* stream;
};


// Consumes a byte from the stream and checks if it's of the expected type
// 
static void match_type(BytecodeStream& stream, unsigned char type)
{
    unsigned char gottype = stream.read_byte();
    if (gottype != type) 
        throw DeserializationError(format_string("Expected type %c, got %c", type, gottype));
}


static BobObject* d_null(BytecodeStream& stream)
{
    (void)stream;

    // Doesn't have to read anything...
    // 
    return new BobNull();
}


static BobObject* d_boolean(BytecodeStream& stream)
{
    unsigned char val = stream.read_byte();
    return new BobBoolean(val == 1);
}


static BobObject* d_number(BytecodeStream& stream)
{
    unsigned word = stream.read_word();
    return new BobNumber(word);
}


static string d_string(BytecodeStream& stream)
{
    unsigned len = stream.read_word();
    return stream.read_string(len);
}


static string d_match_string(BytecodeStream& stream)
{
    match_type(stream, SER_TYPE_STRING);
    return d_string(stream);
}


static BobObject* d_symbol(BytecodeStream& stream)
{
    string str = d_string(stream);
    return new BobSymbol(str);
}


// Forward declarations
// 
static BobObject* d_match_object(BytecodeStream& stream);


static BobObject* d_pair(BytecodeStream& stream)
{
    BobObject* first = d_match_object(stream);
    BobObject* second = d_match_object(stream);
    return new BobPair(first, second);
}


// This function is special: it doesn't return a pointer to BobObject, but
// an instruction, by value. It's called when we know that only instructions
// are expected.
//
static BobInstruction d_match_instruction(BytecodeStream& stream)
{
    match_type(stream, SER_TYPE_INSTR);
    unsigned word = stream.read_word();
    BobInstruction instr(word >> 24, word & 0xFFFFFF);
    return instr;
}


// This macro is used inside d_codeobject to avoid code duplication.
//
#define D_MATCH_SEQUENCE(seq, d_func)               \
    do {unsigned len, i;                            \
        match_type(stream, SER_TYPE_SEQUENCE);      \
        len = stream.read_word();                   \
        for (i = 0; i < len; ++i)                   \
            seq.push_back(d_func(stream));          \
        } while (0);


static BobObject* d_codeobject(BytecodeStream& stream)
{
    BobCodeObject* codeobj = new BobCodeObject();
    codeobj->name = d_match_string(stream);

    D_MATCH_SEQUENCE(codeobj->args, d_match_string);
    D_MATCH_SEQUENCE(codeobj->constants, d_match_object);
    D_MATCH_SEQUENCE(codeobj->varnames, d_match_string);
    D_MATCH_SEQUENCE(codeobj->code, d_match_instruction);

    return codeobj;
}


// Matches and deserializes any Bob object
// 
static BobObject* d_match_object(BytecodeStream& stream)
{
    unsigned char type = stream.read_byte();

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
        case SER_TYPE_CODEOBJECT:
            return d_codeobject(stream);
        default:
            throw DeserializationError(format_string("Expected an object type, got %c", type));
    }

    assert(0 && "Unreachable");
    return 0;
}


BobCodeObject* deserialize_bytecode(const string& filename)
{
    BytecodeStream stream(filename.c_str());

    unsigned magic = stream.read_word();
    if (magic != MAGIC_CONST)
        throw DeserializationError(format_string("Invalid bytecode stream (magic = 0x%0X)", magic));

    match_type(stream, SER_TYPE_CODEOBJECT);
    return static_cast<BobCodeObject*>(d_codeobject(stream));
}

