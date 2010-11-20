//*****************************************************************************
// bob: The virtual machine implementation
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include "vm.h"
#include <cstdio>

using namespace std;


struct VMData
{
    FILE* output_stream;
};


BobVM::BobVM(const string& output_file)
{
    d = new VMData;

    if (output_file == "") 
        d->output_stream = stdout;
    else {
        d->output_stream = fopen(output_file.c_str(), "w");

        if (!d->output_stream)
            throw VMError("Unable to open for output: " + output_file);
    }
}



