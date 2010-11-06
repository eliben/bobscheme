#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include "mem.h"
#include "assert.h"
#include "cutils.h"
#include "dstring.h"


/******************** Private data types ********************/


typedef struct HTEntry_t
{
    void* key;
    void* val;
    struct HTEntry_t* next;
} HTEntry;


/* This structure is used to save some repetitive code when 
** looking for entries in the table. See the code of
** ht_find_entry for details.
*/ 
typedef struct HTEntryAndBucket_t
{
    HTEntry* entry;
    size_t nbucket;
} HTEntryAndBucket;


struct HashTable_t
{
    HTableType type;
    HTEntry** table;
    size_t numbuckets;
    size_t numentries;
};

/******************** Private functions ********************/

static void ht_free_entry(hashtable ht, HTEntry* entry)
{
    if (ht->type.key_free_func)
        ht->type.key_free_func(entry->key);
    if (ht->type.val_free_func)
        ht->type.val_free_func(entry->val);

    mem_free(entry);
}


/* Replace the value of the 'entry' with 'newval'
*/
static void ht_entry_replace_val(hashtable ht, HTEntry* entry, void* newval)
{
    if (ht->type.val_free_func)
        ht->type.val_free_func(entry->val);
    
    if (ht->type.val_copy_func)
        entry->val = ht->type.val_copy_func(newval);
    else
        entry->val = newval;
}


static HTEntry* ht_new_entry(hashtable ht, void* key, void* val)
{
    HTEntry* he = mem_alloc(sizeof(*he));
    
    if (ht->type.key_copy_func)
        he->key = ht->type.key_copy_func(key);
    else
        he->key = key;
    
    if (ht->type.val_copy_func)
        he->val = ht->type.val_copy_func(val);
    else
        he->val = val;
    
    he->next = NULL;
    return he;
}


/* Compute the number of bucket into which 'key' should go in the
** given hashtable.
** If no hash function was specified, the address of the key
** is used instead.
*/
static size_t ht_bucket_for_key(hashtable ht, void* key)
{
    size_t hash;
    
    if (ht->type.hash_func)
        hash = ht->type.hash_func(key);
    else
        hash = (size_t) key;
    
    return hash % ht->numbuckets;
}


static int ht_keys_equal(hashtable ht, void* k1, void* k2)
{
    if (ht->type.keys_equal_func)
        return ht->type.keys_equal_func(k1, k2);
    else
        return k1 == k2;
}


/* Looks for the given key in the hashtable. Returns an instance
** of the HTEntryAndBucket struct.
** If the entry was found, .entry will point to it, and .nbucket
** is the number of the bucket in which the entry was found.
** If the entry wasn't found, .entry will be NULL.
** 
** This is done to save a lot of repetitive coding. All the public
** API functions looking for entries (for finding, removing,
** replacing etc.) can now use this function.
*/
static HTEntryAndBucket ht_find_entry(hashtable ht, void* key)
{
    HTEntry* he;
    HTEntryAndBucket eab = {NULL, 0};
    eab.nbucket = ht_bucket_for_key(ht, key);
    assert(eab.nbucket < ht->numbuckets);
    
    he = ht->table[eab.nbucket];
    
    while (he)
    {
        if (ht_keys_equal(ht, he->key, key))
        {
            eab.entry = he;
            return eab;
        }
        he = he->next;
    }
    
    return eab;
}


/* Compute the number of buckets for the given size hint.
** The algorithm used at the moment: the closest power of 2 
** above the hint.
*/
static size_t ht_calc_numbuckets(size_t sizehint)
{
    /* minimal table size */
    size_t size = 4;
    
    if (sizehint >= LONG_MAX)
        return LONG_MAX;
    
    while (size < sizehint)
        size <<= 1;
    
    return size;
}


static void ht_resize_table(hashtable ht, size_t newsize)
{
    /* Create a new table on the stack (it will be orderly 
    ** deallocated when this function returns).
    */
    struct HashTable_t newtable;
    size_t i;
    
    newtable.numbuckets = newsize;
    newtable.numentries = ht->numentries;
    newtable.type = ht->type;
    newtable.table = mem_alloc(newsize * sizeof(HTEntry*));
    memset(newtable.table, 0, newsize * sizeof(HTEntry*));
    
    /* Move all the entreis from the old table to the new table.
    ** Note: no memory is allocated/deallocated, we simply move
    ** all the pointers.
    */
    for (i = 0; i < ht->numbuckets && ht->numentries > 0; ++i)
    {
        HTEntry* he = ht->table[i];
        HTEntry* nexthe;
        
        while (he)
        {
            size_t newbucket;
            newbucket = ht_bucket_for_key(&newtable, he->key);
            
            nexthe = he->next;
            he->next = newtable.table[newbucket];
            newtable.table[newbucket] = he;
            
            ht->numentries--;
            he = nexthe;
        }
    }
    
    assert(ht->numentries == 0);
    mem_free(ht->table);
    *ht = newtable;
}


/******************** Public API ********************/


hashtable hashtable_new(HTableType type, size_t sizehint)
{
    hashtable ht = mem_alloc(sizeof(*ht));
    
    ht->numbuckets = ht_calc_numbuckets(sizehint);
    ht->type = type;
    ht->numentries = 0;
    ht->table = mem_alloc(ht->numbuckets * sizeof(HTEntry*));
    
    memset(ht->table, 0, ht->numbuckets * sizeof(HTEntry*));
    
    return ht;
}


size_t hashtable_numentries(hashtable ht)
{
    return ht->numentries;
}


void* hashtable_find(hashtable ht, void* key)
{
    HTEntryAndBucket eab = ht_find_entry(ht, key);
    return eab.entry ? eab.entry->val : NULL;
}


HT_RETVAL hashtable_insert(hashtable ht, void* key, void* val, int replace)
{
    HTEntry* he;
    HTEntryAndBucket eab;
     
    eab = ht_find_entry(ht, key);
    
    if (eab.entry)
    {
        if (replace)
        {
            ht_entry_replace_val(ht, eab.entry, val);
            return HT_OK;
        }
        else return HT_ERR_EXISTS;
    }
    
    /* Doesn't exist - so we prepend it to the chain of its
    ** bucket.
    */
    he = ht_new_entry(ht, key, val);
    he->next = ht->table[eab.nbucket];
    ht->table[eab.nbucket] = he;
    ht->numentries++;
    
    /* If there are too many entries in the table, we resize it 
    ** to be twice as large.
    */
    if (ht->numentries > ht->numbuckets)
        ht_resize_table(ht, ht->numbuckets * 2);
    
    return HT_OK;
}


void* hashtable_find_or_insert(hashtable ht, void* key, void* val)
{
    HTEntry* he;
    HTEntryAndBucket eab = ht_find_entry(ht, key);
    
    if (eab.entry)
        return eab.entry->val;
    
    /* Doesn't exist, so we insert and return the value.
    */
    he = ht_new_entry(ht, key, val);
    he->next = ht->table[eab.nbucket];
    ht->table[eab.nbucket] = he;
    ht->numentries++;
    
    /* If there are too many entries in the table, we resize it 
    ** to be twice as large.
    */
    if (ht->numentries > ht->numbuckets)
        ht_resize_table(ht, ht->numbuckets * 2);
    
    return he->val;
}


HT_RETVAL hashtable_replace(hashtable ht, void* key, void* newval)
{
    HTEntryAndBucket eab = ht_find_entry(ht, key);
    
    if (eab.entry)
    {
        ht_entry_replace_val(ht, eab.entry, newval);
        return HT_OK;
    }
    else return HT_ERR_NOTFOUND;
}


HT_RETVAL hashtable_remove(hashtable ht, void* key)
{
    HTEntryAndBucket eab = ht_find_entry(ht, key);
    
    if (eab.entry)
    {
        /* The entry is in the table.
        */
        HTEntry* he = ht->table[eab.nbucket];
        assert(he);
        
        /* Is it the first entry in the bucket?
        */
        if (he == eab.entry)
        {
            ht->table[eab.nbucket] = he->next;
            ht_free_entry(ht, he);
            ht->numentries--;
            return HT_OK;
        }
        
        /* It's not the first. So find its predecessor in the 
        ** chain.
        */
        while (he)
        {
            if (he->next == eab.entry)
            {
                he->next = eab.entry->next;
                ht_free_entry(ht, eab.entry);
                ht->numentries--;
                return HT_OK;
            }
            
            he = he->next;
        }
        
        /* We can't be here, since the entry was found in the
        ** table.
        */
        assert(0);
        return HT_ERR_NOTFOUND; /* soothe compiler warning */
    }
    else return HT_ERR_NOTFOUND;
}


void hashtable_free(hashtable ht)
{
    size_t i;
    
    for (i = 0; i < ht->numbuckets; ++i)
    {
        HTEntry *he, *next;
        
        if ((he = ht->table[i]) == NULL)
            continue;
        
        while (he)
        {
            next = he->next;
            ht_free_entry(ht, he);
            he = next;
        }
    }
    
    mem_free(ht->table);
    mem_free(ht);
}


/******************** Common HTableTypes ********************/


HTableType HTableType_copy_dstring_key_val = {
    /* hash */      ht_hash_dstring,
    /* key dup */   ht_dup_dstring,
    /* val dup */   ht_dup_dstring,
    /* compare */   ht_compare_dstring,
    /* key free */  ht_free_dstring,
    /* val free */  ht_free_dstring
};


HTableType HTableType_copy_dstring_key = {
    /* hash */      ht_hash_dstring,
    /* key dup */   ht_dup_dstring,
    /* val dup */   NULL,
    /* compare */   ht_compare_dstring,
    /* key free */  ht_free_dstring,
    /* val free */  NULL
};


HTableType HTableType_own_dstring_key_val = {
    /* hash */      ht_hash_dstring,
    /* key dup */   NULL,
    /* val dup */   NULL,
    /* compare */   ht_compare_dstring,
    /* key free */  ht_free_dstring,
    /* val free */  ht_free_dstring
};


/* Computes a hash value of a string. 
** Algorithm taken from "The Practice of Programming" by Kernighan
** and Pike, 1st edition, section 2.9
*/ 
size_t ht_hash_dstring(const void* key)
{
    char* cstr = dstring_cstr((dstring) key);
    size_t h = 0;
    
    for (; *cstr; ++cstr)
        h = 37 * h + *cstr;
    return h;    
}


int ht_compare_dstring(const void* key1, const void* key2)
{
    dstring dkey1 = (const dstring) key1;
    dstring dkey2 = (const dstring) key2;
    return strcmp(dstring_cstr(dkey1), dstring_cstr(dkey2)) == 0;
}


void ht_free_dstring(void* dstr)
{
    dstring_free((dstring) dstr);
}


void* ht_dup_dstring(const void* dstr)
{
    return dstring_dup((const dstring) dstr);
}
