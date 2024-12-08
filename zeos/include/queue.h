#ifndef _LINUX_QUEUE_H
#define _LINUX_QUEUE_H

#include "types.h"

typedef struct {
    char* buf;
    uint write_idx;
    uint read_idx;
    const int len;
} keyboard_buffer;

#define KBUF_NEW(length) \
    (keyboard_buffer){ (char[length+1]){0}, 0, 0, length+1}

keyboard_buffer __kbuf_new(char *buffer, const int length);

int kbuf_push(keyboard_buffer *kbuf, char item);

int kbuf_pop(keyboard_buffer *kbuf, char *value);

#define KBUF_ITER(kbuf, value) \
    char __buf__[kbuf.len]; \
    for (int i = 0; i < kbuf.len; i++) __buf__[i] = kbuf.buf[i]; \
    kbuf.buf = __buf__; \
    char value; \
    while (kbuf_pop(&kbuf, &value))

#define KBUF_ITER_CONSUME(kbuf) \
    char value; \
    while (kbuf_pop(&kbuf, &value))

#endif /* _LINUX_QUEUE_H */