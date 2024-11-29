#include <keyboard.h>
#include <list.h>

keyboard_buffer kbuf = KBUF_NEW(32);

struct list_head keyboard_blocked;

void init_keyboard() {
    INIT_LIST_HEAD(&keyboard_blocked);
}

void keyboard_unblock_first() {
    struct list_head *lh = list_first(&keyboard_blocked);
    struct task_struct ts* = list_entry(lh, struct task_struct, list);
    update_process_state_rr(ts, &readyqueue);
}