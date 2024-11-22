/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

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

#endif  /* __LIBC_H__ */
