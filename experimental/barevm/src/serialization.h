//*****************************************************************************
// bob: (De)serialization of Bob code objects
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <stdexcept>


// The exception type thrown by the deserializer
//
struct DeserializationError : public std::runtime_error
{
    DeserializationError(const std::string& reason) 
        : std::runtime_error(reason)
    {}
};


class BobCodeObject;


// Given a bytecode file, deserializes it into a new BobCodeObject
//
BobCodeObject* deserialize_bytecode(const std::string& filename); 

#endif /* SERIALIZATION_H */
