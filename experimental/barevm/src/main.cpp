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
    BobObject* nl = new BobNull();
    BobObject* n8 = new BobNumber(8);
    BobObject* nn8 = new BobNumber(8);
    BobObject* n9 = new BobNumber(9);
    BobObject* br = new BobSymbol("kevr");
    BobObject* boo = new BobBoolean(true);
    BobObject* bo2 = new BobBoolean(true);

    BobObject* pp1 = new BobPair(n8, nl);
    BobObject* pp2 = new BobPair(boo, pp1);
    BobObject* pp3 = new BobPair(br, pp2);
    BobObject* pp4 = new BobPair(pp2, pp3);
    BobObject* pp5 = new BobPair(n8, n9);
    BobObject* pp6 = new BobPair(bo2, pp5);
    BobObject* pp7 = new BobPair(n8, pp4);

    cerr << (n8 == n8) << endl;
    cerr << objects_equal(n8, nn8) << endl;
    cerr << objects_equal(n8, n8) << endl;
    cerr << objects_equal(n9, n8) << endl;
    cerr << objects_equal(n8, nl) << endl;
    cerr << objects_equal(br, nl) << endl;
    cerr << objects_equal(boo, bo2) << endl;

    cerr << pp1->repr() << endl;
    cerr << pp2->repr() << endl;
    cerr << pp3->repr() << endl;
    cerr << pp4->repr() << endl;
    cerr << pp5->repr() << endl;
    cerr << pp6->repr() << endl;
    cerr << pp7->repr() << endl;
    
    return 0;
}
