#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>


/* Return codes used by the hashtable API.
*/
typedef enum 
{
    HT_OK = 0,
    HT_ERR_EXISTS,
    HT_ERR_NOTFOUND
} HT_RETVAL;


/* This HTableType must be specified when a new hashtable is 
** created. It configures the table's operation. By leaving this
** specification to the user, the hashtable can be completely
** generic with respect to types of keys and values and the 
** responsibility for resources. 
** For some useful examples, see the predefined HTableType_*
** objects in hashtable.c
**
** hash_func:
**      Hash function taking a key and returning an integer value 
**      in the range of size_t.
** 
** key_copy_func:
**      If NULL, the table won't copy the keys. Rather, it will
**      just keep pointers to them. Thus, the keys must stay
**      allocated througout the usage of the table.
**      If this function is specified by the user, key_free_func
**      should almost certainly be also specified.
**
** val_copy_func:
**      Same concept as for keys. See key_copy_func above.
**
** keys_equal_func:
**      A function that takes two keys and returns TRUE if they're
**      equal, FALSE otherwise.
**     
** key_free_func:
**      When the table copies the key on insertion, it must have
**      a way to free the key when it's itself being freed. This
**      function privides the means to free keys.
**      It can also be specified without key_copy_func. In this
**      case, the hashtable "takes ownership" of the key and 
**      releases it when freed.
** 
** val_free_func:
**      Same concept as for keys. See val_free_func above.
*/ 
typedef struct HTableType_t
{
    size_t (*hash_func)(const void* key);
    void* (*key_copy_func)(const void* key);
    void* (*val_copy_func)(const void* val);
    int (*keys_equal_func)(const void* key1, const void* key2);
    void (*key_free_func)(void* key);
    void (*val_free_func)(void* val);
} HTableType;


struct HashTable_t;
typedef struct HashTable_t* hashtable;


/**************** Public API ****************/


/* Create a new hashtable with the given type. 
** sizehint:
**      The amount of entries you expect to have in the table.
**      The hashtable will pre-allocate a suitable amount of 
**      space, which will save time expanding later. Note that
**      this is only a hint - the actual size will be chosen by
**      the table. Set to 0 if you want a minimal table.
*/
hashtable hashtable_new(HTableType type, size_t sizehint);

/* Total number of entries (key/value pairs) in the hashtable.
*/
size_t hashtable_numentries(hashtable ht);

/* Find the value corresponding to the given key in the table.
** If found, the value is returned. Otherwise, NULL is returned.
*/
void* hashtable_find(hashtable ht, void* key);

/* Insert a new key/value pair into the hashtable, or replace an
** existing one.
**
** If the given key already exists in the hashtable, the 'replace'
** argument is examined. If it's TRUE (1), the value associated 
** with the key is replaced by 'val' and HT_OK is returned. 
** If 'replace' is FALSE (0), HT_ERR_EXISTS is returned.
** If the key doesn't exist in the table, it is added and HT_OK is
** returned.
*/
HT_RETVAL hashtable_insert(hashtable ht, void* key, void* val, int replace);

/* If the key is in the table, its value is returned. Otherwise,
** the key/value pair is added to the table and the value is 
** returned. Note: when a value-copy function is specified in the
** HTableType, the value returned here will be the new value created
** by that function.
*/
void* hashtable_find_or_insert(hashtable ht, void* key, void* val);

/* Find key in the table and replace its associated value with
** newval.
** If key isn't found, returns HT_ERR_NOTFOUND.
** Otherwise, the value is replaced and HT_OK is returned.
*/
HT_RETVAL hashtable_replace(hashtable ht, void* key, void* newval);

/* Remove the given key from the table. HT_OK is returned if the
** key was found and removed. HT_ERR_NOTFOUND is returned if the
** key doesn't exist.
*/
HT_RETVAL hashtable_remove(hashtable ht, void* key);

/* Free the hash table and all the memory allocated for it. Keys
** and/or values will also be freed if the relevant free functions
** were specified for the hashtable.
*/
void hashtable_free(hashtable ht);


/**************** Some useful HTableTypes ****************/

/* The simplest type of hashtable - both keys and values are 
** dstrings, copied by the table.
*/
extern HTableType HTableType_copy_dstring_key_val;


/* The hashtable keys are dstrings, copied by the table. Values
** are of an arbitrary user-defined type.
*/
extern HTableType HTableType_copy_dstring_key;


/* This type of hashtable takes ownership of the dstring keys and
** values provided by the user. 
** Meaning: it doesn't copy them on insertion, but frees them when
** it's freed itself.
*/
extern HTableType HTableType_own_dstring_key_val;


/* Some useful utilities for constructing hashtables with dstring
** keys and values.
*/
size_t ht_hash_dstring(const void* key);
int ht_compare_dstring(const void* key1, const void* key2);
void ht_free_dstring(void* dstr);
void* ht_dup_dstring(const void* dstr);


#endif /* HASHTABLE_H */
