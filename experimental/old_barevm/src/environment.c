/******************************************************************************
** bob: Environment - implementation of name->value mapping hierarchical 
** environments.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "mem.h"
#include "hashtable.h"
#include "environment.h"


/* Bindings of each env are kept in a hashtable. The keys are variable 
** names which are interned strings. Therefore, the hashtable should 
** not manage the memory of keys, just use their values. Moreover, since
** the strings are interned, the keys can be compared and hashed as 
** simple pointers.
*/
static size_t env_hash_key(const void* dstr)
{
    return (size_t) dstr;
}


static int env_keys_equal(const void* dstr1, const void* dstr2)
{
    return dstr1 == dstr2;
}


/* The memory for keys and values isn't managed by the environment.
*/
HTableType HTableType_for_envs = {
    /* hash */      env_hash_key,
    /* key dup */   NULL,
    /* val dup */   NULL,
    /* compare */   env_keys_equal,
    /* key free */  NULL,
    /* val free */  NULL
};


struct BobEnv {
    hashtable bindings;
    struct BobEnv* parent;
};


BobEnv* BobEnv_new(BobEnv* parent)
{
    /* Environment bindings are expected to be short, so we create a 
    ** small hashtable. It should do in most cases. In others, the
    ** hashtable will just grow automatically.
    */
    BobEnv* env = mem_alloc(sizeof(*env));
    env->bindings = hashtable_new(HTableType_for_envs, 8);
    env->parent = parent;
    return env;
}


void BobEnv_free(BobEnv* env)
{
    hashtable_free(env->bindings);
    mem_free(env);
}


BobObject* BobEnv_lookup(BobEnv* env, dstring var)
{
    BobObject* val = hashtable_find(env->bindings, var);
    if (val) 
        return val;
    else {
        if (env->parent)
            return BobEnv_lookup(env->parent, var);
        else
            return NULL;
    }
}


BobObject* BobEnv_set_var_val(BobEnv* env, dstring var, BobObject* val)
{
    if (hashtable_replace(env->bindings, var, val) == HT_OK)
        return val;
    else {
        if (env->parent)
            return BobEnv_set_var_val(env->parent, var, val);
        else
            return NULL;
    }
}


void BobEnv_define_var(BobEnv* env, dstring var, BobObject* val)
{
    hashtable_insert(env->bindings, var, val, TRUE);
}
