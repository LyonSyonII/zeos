#include "types.h"

typedef struct {
    void* buf;
    uint ptr_size;
    uint len;
    uint start;
    uint end;
} Queue;

Queue __init_queue(void* base, uint ptr_size, uint len);

#define DEFINE_QUEUE(name, type) \
    typedef struct { \
        type* buf; \
        uint capacity; \
        uint start; \
        uint end; \
        uint len; \
    } name; \
    \
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
    int name##_push(name* queue, type value) { return __queue_push(queue, &value, sizeof(type)); } \
    int name##_push_front(name* queue, type value) { return __queue_push_front(queue, &value, sizeof(type)); } \
    int name##_pop(name* queue, type* out_value) { return __queue_pop(queue, out_value, sizeof(type)); } \
    int name##_pop_back(name* queue, type* out_value) { return __queue_pop_front(queue, out_value, sizeof(type)); }\
    void name##_clear(name* queue) { queue->start = queue->end = queue->len = 0; } \
    int name##_len(name* queue) { return queue->len; } \
    int name##is_empty(name* queue) { return __queue_is_empty(queue); } \
    int name##is_full(name* queue) { return __queue_is_full(queue); }

int __queue_push(void* q, void* elem, int size);
int __queue_push_front(void* q, void* elem, int size);
int __queue_pop(void* q, void* out, int size);
int __queue_pop_back(void* q, void* out, int size);
int __queue_is_empty(void* q);
int __queue_is_full(void* q);

DEFINE_QUEUE(charq, int);