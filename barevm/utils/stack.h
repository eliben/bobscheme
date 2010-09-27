#ifndef STACK_H
#define STACK_H


struct Stack_t;
typedef struct Stack_t* stack;


/* This StackType must be specified when a new stack is created. It configures
** the stack's operation. See the section "Some useful StackTypes" below" for
** examples.
**
** item_free_func:
**      When the stack is freed, this function will be called for each item.
**      customize the stack's ownership of items and the function used to clean
**      them up. 
**      If NULL, it won't be called.
*/
typedef struct StackType_t
{
    void (*item_free_func)(void* item);
} StackType;


/* Create a new, empty stack.
*/
stack stack_new(StackType type);

/* Push a new item onto the top of the stack.
*/
void stack_push(stack st, void* item);

/* Pop and return an item from the top of the stack. If the stack is empty, a
** null pointer is returned.
*/
void* stack_pop(stack st);

/* Get the value at the top of the stack (without popping it). If the stack is
** empty, a null pointer is returned. 
*/
void* stack_top(stack st);

/* Destroy the stack
*/
void stack_free(stack st);


/************************* Some useful StackTypes *************************/

/* This stack will not free items when destroyed.
*/
extern StackType StackType_no_free;

/* This stack will call mem_free on an item when destroyed.
*/
extern StackType StackType_free_items;


#endif /* STACK_H */
