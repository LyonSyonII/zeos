/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <sched.h>

/** Screen functions **/
/**********************/

#define FD_SCREEN 1
#define FD_BOCHS 2

#define NUM_COLUMNS 80
#define NUM_ROWS    25

extern Byte x, y;

extern Byte screenColor;

Byte inb (unsigned short port);
void printc(char c, int fd);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string, int fd);

void setCursor(int nx, int ny);

void printkint(int i, int fd);
void printkintln(int i, int fd);
void printkhex(int i, int fd);
void printkhexln(int i, int fd);
void printkptr(const void* ptr, int fd);
void printkptrln(const void* ptr, int fd);

// Supports: `%d`, `%p`, `%x`, `%s`.
#define printkf(template, ...) __printkf(template, (const void*[]) { __VA_ARGS__ }, FD_BOCHS)
#define printscreen(template, ...) __printkf(template, (const void*[]) { __VA_ARGS__ }, FD_SCREEN)
void __printkf(const char* template, const void* args[], int fd);

#if defined(DEBUG) && DEBUG > 0
    #define dbg(...) printkf(__VA_ARGS__)
#else
    #define dbg(template, ...) __dummy(template, (const void*[]) { __VA_ARGS__ })
#endif

#endif  /* __IO_H__ */
