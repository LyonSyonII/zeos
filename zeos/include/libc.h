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

struct sem_t { Byte DONOTACCESSTHISSTRUCTORYOUWILLDIE; };

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

// Create an initial semaphore with an initial counter of initial_value; 
// 
// The returned `sem_t` is unusable from user space.
struct sem_t* semCreate(int initial_value);

// Decrement the semaphore’s counter and block the current thread if the counter is negative
int semWait(struct sem_t* s);

// Increase the semaphores's counter and unblock the first blocked thread in the semaphore's queue
int semSignal(struct sem_t* s);

// Destroy the semaphore (only the thread that created a semaphore can destroy it)
int semDestroy(struct sem_t* s);

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
