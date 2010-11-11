//*****************************************************************************
// bob: base BobObject
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef BOBOBJECT_H
#define BOBOBJECT_H


// Abstract base class for all objects managed by the Bob VM
//
class BobObject 
{
public:
    BobObject();
    virtual ~BobObject();

private:
    /* data */  
};



#endif /* BOBOBJECT_H */
