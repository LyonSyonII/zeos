#include <queue.h>

typedef struct {
    char* buf;
    int capacity;
    int start;
    int end;
    int len;
} __dummy_queue;

void copy_element(void *restrict start, void *restrict dest, unsigned long size) {
  DWord *p = start;
  DWord *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size--;
  }
}

void idx_dec(const __dummy_queue* queue, int* idx) {
    *idx -= 1;
    if (*idx < 0) {
        *idx = queue->capacity-1;
    }
}
void idx_inc(const __dummy_queue* queue, int* idx) {
    *idx += 1;
    if (*idx >= queue->capacity) {
        *idx = 0;
    }
}

int __queue_push(void *restrict q, void *restrict elem, int size) {
    __dummy_queue* queue = q;
    
    copy_element(elem, &queue->buf[queue->end], size);
    idx_inc(queue, &queue->end);
    queue->len += 1;

    return 0;
}
int __queue_push_front(void *restrict q, void *restrict elem, int size) {
    __dummy_queue* queue = q;

    copy_element(elem, &queue->buf[queue->start], size);
    idx_dec(queue, &queue->start);
    queue->len += 1;
    
    return 0;
}
int __queue_pop(void *restrict q, void *restrict out, int size) {
    __dummy_queue* queue = q;
    if (__queue_is_empty(q)) return 0;
    
    copy_element(queue->buf + size*queue->start, out, size);
    idx_inc(queue, &queue->start);
    
    return 0;
}
int __queue_pop_back(void *restrict q, void *restrict out, int size) {
    __dummy_queue* queue = q;
    if (__queue_is_empty(q)) return 0;

    idx_dec(queue, &queue->end);
    copy_element(queue->buf + size*queue->end, out, size);
    
    return 0;
}

int __queue_is_empty(void *q) {
    __dummy_queue* queue = q;
    return queue->len == 0 && queue->capacity > 0;
}

int __queue_is_full(void *q) {
    __dummy_queue* queue = q;
    return queue->len == queue->capacity;
}