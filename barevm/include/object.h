/******************************************************************************
** bob: Scheme objects for bare VM
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef OBJECT_H
#define OBJECT_H

#include "dstring.h"


typedef enum BobObjectType {
    TYPE_NULL, TYPE_NUMBER, TYPE_SYMBOL, TYPE_PAIR,
} BobObjectType;


typedef struct BobObject {
    BobObjectType type;

    union {
        int num;
        dstring sym;
        struct {struct BobObject *first, *second;} pair;
    } d;
} BobObject;


BobObject* BobNull_new();
BobObject* BobNumber_new(int num);
BobObject* BobSymbol_new(dstring sym);
BobObject* BobPair_new(BobObject* first, BobObject* second);


#endif /* OBJECT_H */
