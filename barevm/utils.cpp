//*****************************************************************************
// Generic C++ utilities
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************

#include "utils.h"
#include <cstdio>
#include <cstdarg>
#include <cassert>
using namespace std;


string format_string(const char* format, ...)
{
    int len = 64;

    // Since there's no way telling in advance how long the formatted string
    // will be, we'll allocate increasingly large buffers until it fits in.
    // A sentinel '\0' is placed at the end of the buffer. If it's 
    // overwritten by vsnprintf, it means the buffer isn't large enough.
    // Note #1: it would be nice to rely on the return value of vsnprintf, 
    // instead of a sentinel, but MSVC's vsnprintf isn't ISO C conformant, and
    // we need a portable solution.
    // Note #2: realistically, the vast majority of format strings are small 
    // enough to fit into the initial buffer, so no reallocation will be 
    // required.
    //
    while (true) {
        char* buf = new char[len];
        buf[len - 2] = '\0';

        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, len, format, ap);
        va_end(ap);

        if (buf[len - 2] != '\0') {
            delete[] buf;
            len *= 2;
        }
        else {
            return buf;
        }
    }
}

