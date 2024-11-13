#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void keyboard_routine();
void keyboard_handler();

extern char char_map[];

extern struct list_head keyboard_blocked;   // PARCIAL 1
extern char char_read;                      // PARCIAL 1
void block_for_keyboard(void);              // PARCIAL 1
void unblock_first(void);                   // PARCIAL 1

#endif /* __KEYBOARD_H__ */
