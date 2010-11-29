//*****************************************************************************
// Generic C++ utilities
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//*****************************************************************************
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>


// Convert some value to a string. This value must be of a class that supports
// operator<< to a stream. Otherwise, a compile error will be generated.
//
template <class T>
std::string value_to_string(const T& value)
{
    std::stringstream out;
    out << value;
    return out.str();
}


// Create a string from a printf-like format and arguments
//
std::string format_string(const char* format, ...);

#endif /* UTILS_H */
