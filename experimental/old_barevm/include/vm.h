/******************************************************************************
** bob: The main VM implementation
**
** Eli Bendersky (eliben@gmail.com)
** This code is in the public domain
******************************************************************************/
#ifndef VM_H
#define VM_H


struct BobCodeObject;

void init_vm(FILE* output_stream);
void vm_run_code(struct BobCodeObject* codeobj);




#endif /* VM_H */
