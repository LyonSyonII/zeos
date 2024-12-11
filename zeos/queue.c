#include <queue.h>

int kbuf_push(keyboard_buffer *kbuf, char item) {
  if ((kbuf->write_idx + 1) % kbuf->len == kbuf->read_idx) {
    // kbuf->read_idx = (kbuf->read_idx + 1) % kbuf->len;
    return 0;
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