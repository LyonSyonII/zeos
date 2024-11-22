#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void copy_data(void *start, void *dest, int size);
int copy_from_user(void *start, void *dest, int size);
int copy_to_user(void *start, void *dest, int size);

#define VERIFY_READ	0
#define VERIFY_WRITE	1
int access_ok(int type, const void *addr, unsigned long size);

#define min(a,b)	(a<b?a:b)

unsigned long get_ticks(void);

void memset(void *s, unsigned char c, int size);

// custom

typedef struct {
    void* buf;
    uint ptr_size;
    uint len;
    uint start;
    uint end;
} Queue;

Queue __init_queue(void* base, uint ptr_size, uint len);

#define DEFINE_QUEUE(name, length, type) \
    typedef struct { \
        type buf[length]; \
        uint len; \
        uint start; \
        uint end; \
    } name; \
    \
    int push(type value); \
    int pop(type* value);
#endif

DEFINE_QUEUE(CharQueue, 200, char);

CharQueue queue;