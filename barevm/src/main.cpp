//*****************************************************************************
// bob: main 
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include <iostream>
#include "basicobjects.h"
#include "bobobject.h"
#include "utils.h"
#include "bytecode.h"
#include "serialization.h"
#include "vm.h"

using namespace std;

// Set these for debugging or testing the garbage collector.
// A high threshold means the GC won't actually run in the tests.
//
const bool GC_DEBUGGING = false;
const size_t GC_SIZE_THRESHOLD = 20 * 1024 * 1024;


int main(int argc, const char* argv[])
{
    string filename;
    if (argc > 1)
        filename = argv[1];
    else {
        cerr << "Expecting a .bobc file as argument\n";
        return 1;
    }

    try {
        BobCodeObject* bco = deserialize_bytecode(filename);
        BobVM vm;
        BobAllocator::get().set_debugging(GC_DEBUGGING);
        vm.set_gc_size_threshold(GC_SIZE_THRESHOLD);
        vm.run(bco);
    }
    catch (const DeserializationError& err) {
        cerr << "Deserialization ERROR: " << err.what() << endl;
        return 1;
    }
    catch (const VMError& err) {
        cerr << "VM ERROR: " << err.what() << endl;
        return 1;
    }
    
    return 0;
}
