/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include "types.h"
#include <stats.h>

#define STDOUT 1

extern int errno;

typedef struct { } sem_t;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int gettime();

int getpid();

int fork();

void exit();

int yield();

int getKey(char* b, int timeout);

int gotoXY(int x, int y);

int changeColour(int fg, int bg);

int clrscr(char* b);

int threadCreateWithStack(void (*function)(void *arg), int N, void *parameter);

sem_t* semCreate(int initial_value);

int semWait(sem_t* s);

int semSignal(sem_t* s);

int semDestroy(sem_t* s);

char* memRegGet(int num_pages);

int memRegDel(char* m);

int get_stats(int pid, struct stats *st);

void SAVE_REGS(void);
void RESTORE_REGS(void);

// custom

// Prints the provided buffer.
int print(const char* buffer);
// Prints the provided character.
int printchar(char c);
// Prints the provided integer.
int printint(int i);
// Prints the provided integer with a newline at the end.
int printintln(int i);
// Prints the provided buffer with a newline at the end.
int println(const char* buffer);
// Supports: `%d`, `%p`, `%x`, `%s`.
#define printf(template, ...) __printf(template, (const void*[]) { __VA_ARGS__ })
void __printf(const char* template, const void* args[]);

#endif  /* __LIBC_H__ */
