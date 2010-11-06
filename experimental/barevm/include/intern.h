/******************************************************************************
** bob: Implementation of interned strings.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef INTERN_H
#define INTERN_H

#include "dstring.h"

/* Implementation of interning of immutable dynamic strings. Keeps an 
** internal table for saving strings, and returns pointers to them. These 
** pointers can be used to uniquely represent the string, without worrying 
** about allocation and deallocation. The table keeps each string only once.
*/

/* Intern the given string - return a unique dstring representing it.
** Note: the function doesn't assume ownership of the passed 'str', but
** makes its own copy.
*/
dstring intern_dstring(dstring str);

/* Return an interned dstring representing the given c-string.
*/
dstring intern_cstring(const char* cstr);

/* Cleanup of interning data structures.
*/
void intern_cleanup();


#endif /* INTERN_H */

