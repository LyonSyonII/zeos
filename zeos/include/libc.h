/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include "types.h"
#include <stats.h>

#define FD_BOCHS 2
#define FD_SCREEN 1

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

int changeColor(int fg, int bg);

int clrscr(char* b);

int threadCreateWithStack(void (*function)(void* arg), int N, void* parameter);

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
int print(char* buffer, int fd);
// Prints the provided character.
int printchar(char c, int fd);
// Prints the provided integer.
int printint(int i, int fd);
// Prints the provided integer with a newline at the end.
int printintln(int i, int fd);
// Prints the provided buffer with a newline at the end.
int println(char* buffer, int fd);
// Supports: `%d`, `%p`, `%x`, `%s`.
#define printf(template, ...) __printf(template, (const void*[]){__VA_ARGS__}, FD_BOCHS)
#define printscreen(template, ...) __printf(template, (const void*[]){__VA_ARGS__}, FD_SCREEN)
/* #define printf(template, ...) __printf(template, CREATE_ARRAY(__VA_ARGS__), 2)
#define printscreen(template, ...) __printf(template, CREATE_ARRAY(__VA_ARGS__), 1) */
void __printf(char* template, const void* args[], int fd);


// Macros per fer el printf sense referencies
#define CAST_0()
#define CAST_1(a1) (void *)(long)(a1)
#define CAST_2(a1, a2) CAST_1(a1), (void *)(long)(a2)
#define CAST_3(a1, a2, a3) CAST_2(a1, a2), (void *)(long)(a3)
#define CAST_4(a1, a2, a3, a4) CAST_3(a1, a2, a3), (void *)(long)(a4)
#define CAST_5(a1, a2, a3, a4, a5) CAST_4(a1, a2, a3, a4), (void *)(long)(a5)
// Extend up to CAST_10 as needed

// Step 2: Macro to count the number of arguments (up to 10)
#define GET_10TH_ARG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10, N, ...) N
#define COUNT_ARGS(...) GET_10TH_ARG(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)

// Step 3: Macro concatenation helpers
#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  arg1##arg2

#define SELECT_CAST_MACRO(count) CONCATENATE(CAST_, count)

// Step 4: Final CREATE_ARRAY macro
#define CREATE_ARRAY(...) \
    (const void *[]){ SELECT_CAST_MACRO(COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__) }


#endif  /* __LIBC_H__ */
