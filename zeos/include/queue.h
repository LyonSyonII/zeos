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


typedef struct {
    char* buf;
    int capacity;
    int start;
    int end;
    int len;
} __dummy_queue;

#define DEFINE_QUEUE(name, type) \
    typedef struct { \
        type* buf; \
        uint capacity; \
        uint start; \
        uint end; \
        uint len; \
    } name; \
    \
    /** Creates a new queue with the provided buffer `base` and `capacity` */ \
    name name_##new(void* base, int capacity); \
    /** Adds a new value to the end of the queue. */ \
    int name##_push(name* queue, type value); \
    /** Adds a new value to the start of the queue. */ \
    int name##_push_front(name* queue, type value); \
    /** Removes a value from the start of the queue and returns it into `out_value`. */ \
    int name##_pop(name* queue, type* out_value); \
    /** Removes a value from the end of the queue and returns it into `out_value`. */ \
    int name##_pop_back(name* queue, type* out_value); \
    /** Empties the queue. */ \
    void name##_clear(name* queue); \
    /** Returns the length of the queue. */ \
    int name##_len(name* queue);
    
#define IMPL_QUEUE(name, type) \
    name name##_new(void* base, uint capacity) { __dummy_queue q = __init_queue(base, capacity); return *(name*)&q; } \
    int name##_push(name* queue, type value) { return __queue_push(queue, &value, sizeof(type)); } \
    int name##_push_front(name* queue, type value) { return __queue_push_front(queue, &value, sizeof(type)); } \
    int name##_pop(name* queue, type* out_value) { return __queue_pop(queue, out_value, sizeof(type)); } \
    int name##_pop_back(name* queue, type* out_value) { return __queue_pop_back(queue, out_value, sizeof(type)); }\
    void name##_clear(name* queue) { queue->start = queue->end = queue->len = 0; } \
    int name##_len(name* queue) { return queue->len; } \
    int name##_is_empty(name* queue) { return __queue_is_empty(queue); } \
    int name##_is_full(name* queue) { return __queue_is_full(queue); }


__dummy_queue __init_queue(void *base, uint len);
int __queue_push(void* q, void* elem, int size);
int __queue_push_front(void* q, void* elem, int size);
int __queue_pop(void* q, void* out, int size);
int __queue_pop_back(void* q, void* out, int size);
int __queue_is_empty(void* q);
int __queue_is_full(void* q);

#endif /* _LINUX_QUEUE_H */