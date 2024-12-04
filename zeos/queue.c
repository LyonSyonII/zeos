#include <queue.h>

int kbuf_push(keyboard_buffer *kbuf, char item) {
  if ((kbuf->write_idx + 1) % kbuf->len == kbuf->read_idx) {
    kbuf->read_idx = (kbuf->read_idx + 1) % kbuf->len;
    // return 0;
  }
  kbuf->buf[kbuf->write_idx] = item;
  kbuf->write_idx = (kbuf->write_idx + 1) % kbuf->len;
  return 1;
}
int kbuf_pop(keyboard_buffer *kbuf, char *value) {
  if (kbuf->read_idx == kbuf->write_idx) {
    return 0;
  }

  *value = kbuf->buf[kbuf->read_idx];
  kbuf->read_idx = (kbuf->read_idx + 1) % kbuf->len;
  return 1;
}







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

__dummy_queue __init_queue(void *base, uint len) {
    __dummy_queue q = {
        .buf = base,
        .capacity = len,
        .start = 0,
        .end = 0,
        .len = 0,
    };
    return q;
}

int __queue_push(void *restrict q, void *restrict elem, int size) {
    __dummy_queue* queue = q;
    
    copy_element(elem, &queue->buf[queue->end], size);
    idx_inc(queue, &queue->end);

    if (!__queue_is_full(&queue)) queue->len += 1;

    return 1;
}
int __queue_push_front(void *restrict q, void *restrict elem, int size) {
    __dummy_queue* queue = q;

    copy_element(elem, &queue->buf[queue->start], size);
    idx_dec(queue, &queue->start);
    queue->len += 1;
    
    return 1;
}
int __queue_pop(void *restrict q, void *restrict out, int size) {
    __dummy_queue* queue = q;
    if (__queue_is_empty(q)) return 0;
    
    copy_element(queue->buf + size*queue->start, out, size);
    idx_inc(queue, &queue->start);
    queue->len -= 1;
    
    return 1;
}
int __queue_pop_back(void *restrict q, void *restrict out, int size) {
    __dummy_queue* queue = q;
    if (__queue_is_empty(q)) return 0;

    idx_dec(queue, &queue->end);
    copy_element(queue->buf + size*queue->end, out, size);
    queue->len -= 1;

    return 1;
}

int __queue_is_empty(void *q) {
    __dummy_queue* queue = q;
    return queue->len == 0 && queue->capacity > 0;
}

int __queue_is_full(void *q) {
    __dummy_queue* queue = q;
    return queue->len == queue->capacity;
}