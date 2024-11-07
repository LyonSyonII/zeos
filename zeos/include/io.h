/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <sched.h>

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);

void printkint(int i);
void printkintln(int i);
void printkhex(int i);
void printkhexln(int i);
void printkptr(const void* ptr);
void printkptrln(const void* ptr);

// Supports: `%d`, `%p`, `%x`, `%s`.
#define printf(template, ...) __printf(template, (const void*[]) { __VA_ARGS__ })
void __printf(const char* template, const void* args[]);
void dbg_task(struct task_struct* task);

#endif  /* __IO_H__ */
