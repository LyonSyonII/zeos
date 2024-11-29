#include <keyboard.h>
#include <list.h>
#include <queue.h>
#include <sched.h>

keyboard_buffer kbuf = KBUF_NEW(32);

struct list_head keyboard_blocked;

void init_keyboard() {
    INIT_LIST_HEAD(&keyboard_blocked);
}

void keyboard_unblock_first() {
    if (list_empty(&keyboard_blocked)) return;
    
    struct list_head *lh = list_first(&keyboard_blocked);
    struct task_struct *ts = list_entry(lh, struct task_struct, list);
    update_process_state_rr(ts, &readyqueue);
}


void keyboard_update_blocked() {
    struct list_head *lh, *element, *n;
    list_for_each_safe(element, n, &keyboard_blocked) {
        struct task_struct *ts = list_entry(element, struct task_struct, list);
        if (--(ts->p_stats.blocked_ticks) <= 0) update_process_state_rr(ts, &readyqueue);
    }
}