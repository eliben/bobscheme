//*****************************************************************************
// bob: main 
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#include <iostream>
#include "basicobjects.h"

using namespace std;


int main(int argc, const char* argv[])
{
    BobNull* nl = new BobNull();
    BobNumber* n8 = new BobNumber(8);
    BobNumber* nn8 = new BobNumber(8);
    BobNumber* n9 = new BobNumber(9);


    cerr << (n8 == n8) << endl;
    cerr << objects_equal(n8, nn8) << endl;
    cerr << objects_equal(n8, n8) << endl;
    cerr << objects_equal(n9, n8) << endl;
    cerr << objects_equal(n8, nl) << endl;

    
    return 0;
}
