#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

extern keyboard_buffer kbuf;

extern struct list_head keyboard_blocked;

void init_keyboard();

void keyboard_unblock_first();

void keyboard_update_blocked();

#endif /* __KEYBOARD_H__ */
