/******************************************************************************
** bob: The main VM implementation
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef VM_H
#define VM_H


typedef struct BobVMState BobVMState;


BobVMState* create_vm(FILE* output_stream);


#endif /* VM_H */
