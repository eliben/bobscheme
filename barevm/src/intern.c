/******************************************************************************
** bob: Implementation of interned strings.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "intern.h"
#include "hashtable.h"


/* This is a hash table storing for each interned dstring itself as 
** the value. Since it copies keys and values, these will be different
** objects in memory, but equal to each other. The _value_ dstring is
** the unique pointer used by interning.
*/
static hashtable table;


static void create_new_table()
{
   table = hashtable_new(HTableType_copy_dstring_key_val, 100); 
}


dstring intern_dstring(dstring str)
{
    dstring res;

    if (!table)
        create_new_table();

    res = (dstring) hashtable_find_or_insert(table, str, str);
    return res;
}


dstring intern_cstring(const char* cstr)
{
    dstring dstr = dstring_new(cstr);
    dstring interned = intern_dstring(dstr);
    dstring_free(dstr);
    return interned;
}

void intern_cleanup()
{
    hashtable_free(table);
    table = 0;
}
