#include "mem.h"
#include "stack.h"


typedef struct StackEntry_t
{
    void* item;
    struct StackEntry_t* next;
} StackEntry;


/* Implemented as a singly-linked list of items, so all we keep is the head 
** pointer.
*/
struct Stack_t
{
    StackEntry* head;
    StackType type;
};


stack stack_new(StackType type)
{
    stack st = mem_alloc(sizeof(*st));
    st->head = NULL;
    st->type = type;
    return st;
}


void stack_push(stack st, void* item)
{
    StackEntry* entry = mem_alloc(sizeof(*entry));
    entry->item = item;
    entry->next = st->head;
    st->head = entry;
}


void* stack_pop(stack st)
{
    if (st->head)
    {
        void* item = st->head->item;
        StackEntry* next = st->head->next;
        mem_free(st->head);
        st->head = next;
        return item;
    }
    else
        return NULL;
}


void* stack_top(stack st)
{
    if (st->head)
        return st->head->item;
    else
        return NULL;
}


static void stack_free_entry(stack st, StackEntry* entry)
{
    if (st->type.item_free_func)
        st->type.item_free_func(entry->item);
    
    mem_free(entry);    
}


void stack_free(stack st)
{
    StackEntry* entry = st->head;
    
    while (entry)
    {
        StackEntry* next = entry->next;
        stack_free_entry(st, entry);
        entry = next;
    }
    
    mem_free(st);
}


/************************* Some useful StackTypes *************************/


StackType StackType_no_free = {
    /* item_free_func */ NULL
};

StackType StackType_free_items = {
    /* item_free_func */ mem_free
};

