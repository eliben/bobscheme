/******************************************************************************
** bob: Bytecode - code objects, instructions and deserialization.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include <stdio.h>
#include "mem.h"
#include "bytecode.h"
#include "cutils.h"


const int SER_TYPE_NULL        = '0';
const int SER_TYPE_BOOLEAN     = 'b';
const int SER_TYPE_STRING      = 's';
const int SER_TYPE_SYMBOL      = 'S';
const int SER_TYPE_NUMBER      = 'n';
const int SER_TYPE_PAIR        = 'p';
const int SER_TYPE_INSTR       = 'i';
const int SER_TYPE_SEQUENCE    = '[';
const int SER_TYPE_CODEOBJECT  = 'c';


static BOOL d_match_type(FILE* stream, int type)
{
    if (fgetc(stream) == type)
        return TRUE;
    else
        return FALSE;
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
    
}
