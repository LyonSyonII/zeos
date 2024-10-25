/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void page_fault_handler2(int error);

void syscall_handler();

void writeMsr(int msr, int data);

#endif  /* __ENTRY_H__ */
