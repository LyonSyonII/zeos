#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void keyboard_routine();
void keyboard_handler();

extern char char_map[];
extern struct list_head keyboard_blocked;   // PARCIAL

void block_for_keyboard(void);              // PARCIAL
void unblock_first(void);                   // PARCIAL

#endif /* __KEYBOARD_H__ */
