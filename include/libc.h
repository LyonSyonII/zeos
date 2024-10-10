/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include "stats.h"


void itoa(int a, char *b);

int strlen(const char *a);

void perror(void);

int write(int fd, const char *buffer, int size);

int gettime();

int getpid();

int fork();

void exit();

/// Custom methods

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

#endif  /* __LIBC_H__ */
