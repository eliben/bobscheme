/******************************************************************************
** bob: Implementation of interned strings.
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#include "intern.h"
#include "hashtable.h"


/* This is a hash table storing for each interned dstring itself as 
** the value.
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


void intern_cleanup()
{
    hashtable_free(table);
    table = 0;
}
