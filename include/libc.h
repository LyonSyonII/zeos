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

#endif  /* __LIBC_H__ */
