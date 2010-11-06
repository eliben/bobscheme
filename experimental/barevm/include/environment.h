/******************************************************************************
** bob: Environment - implementation of name->value mapping hierarchical 
** environments.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "object.h"


typedef struct BobEnv BobEnv;


/* Create a new env, linked to a parent env. Pass NULL to parent if this
** is a top-level env.
*/
BobEnv* BobEnv_new(BobEnv* parent);

/* Lookup the value of 'var' in the env. Return NULL if no binding found
** in this env or its parent.
*/
BobObject* BobEnv_lookup(BobEnv* env, dstring var);

/* Set the value of 'var' to 'val'. Return 'val' if 'var' was bound,
** NULL if it wasn't.
*/
BobObject* BobEnv_set_var_val(BobEnv* env, dstring var, BobObject* val);

/* Bind 'var' to 'val' in the given env. If a binding for 'var' already
** exists, it's replaced.
*/
void BobEnv_define_var(BobEnv* env, dstring var, BobObject* val);

/* Cleanup an environment.
*/
void BobEnv_free(BobEnv* env);


#endif /* ENVIRONMENT_H */
