/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void page_fault_handler2(int error);

void syscall_handler();

void writeMsr(int msr, int data);


//#################//
//### PARCIAL 1 ###//
//#################//

void read_system_call_handler(); // PARCIAL 1

void waitpid_system_call_handler(); // PARCIAL 2


#endif  /* __ENTRY_H__ */
