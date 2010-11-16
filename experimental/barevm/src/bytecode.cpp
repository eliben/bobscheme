//*****************************************************************************
// bob: Bytecode objects and deserialization
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************

#include "bytecode.h"
#include "basicobjects.h"
#include "utils.h"
#include <cstdio>

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


static string d_expect_string(BytecodeStream& stream)
{
    match_type(stream, SER_TYPE_STRING);
    return d_string(stream);
}


static BobObject* d_symbol(BytecodeStream& stream)
{
    string str = d_string(stream);
    return new BobSymbol(str);
}




