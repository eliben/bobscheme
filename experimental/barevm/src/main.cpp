//*****************************************************************************
// bob: main 
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include <iostream>
#include "basicobjects.h"
#include "utils.h"
#include "bytecode.h"
#include "serialization.h"
#include "vm.h"

using namespace std;


int main(int argc, const char* argv[])
{
    (void)argc;
    string filename = argv[1];

    try {
        BobCodeObject* bco = deserialize_bytecode(filename);
        cerr << bco->repr() << endl;

        BobVM vm;
        vm.run(bco);
    }
    catch (const DeserializationError& err) {
        cerr << "Deserialization ERROR: " << err.what() << endl;
    }
    catch (const VMError& err) {
        cerr << "VM ERROR: " << err.what() << endl;
    }

    
    return 0;
}
