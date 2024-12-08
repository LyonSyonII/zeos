/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <sched.h>

/** Screen functions **/
/**********************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

extern Byte x, y;

extern Byte screenColor;

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);

Byte screenColor; // Color amb el que pintar nous caracters

void setCursor(int nx, int ny);

void printkint(int i);
void printkintln(int i);
void printkhex(int i);
void printkhexln(int i);
void printkptr(const void* ptr);
void printkptrln(const void* ptr);

// Supports: `%d`, `%p`, `%x`, `%s`.
#define printkf(template, ...) __printkf(template, (const void*[]) { __VA_ARGS__ })
void __printkf(const char* template, const void* args[]);
void dbg_task(struct task_struct* task);

void __printkf(const char* template, const void* args[]);
void __dummy(const char* template, const void* args[]);

#if defined(DEBUG) && DEBUG > 0
    #define dbg(...) printkf(__VA_ARGS__)
#else
    #define dbg(template, ...) __dummy(template, (const void*[]) { __VA_ARGS__ })
#endif

#endif  /* __IO_H__ */
