#include "io.h"
#include "list.h"
#include "sched.h"
#include "types.h"
#include <keyboard.h>

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
                   '9',  '0',  '\'', '\0', '\0', '\0', 'q',  'w',  'e',  'r',
                   't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
                   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  '\0',
                   '\0', '\0', '\0', '\0', 'z',  'x',  'c',  'v',  'b',  'n',
                   'm',  ',',  '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                   '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

void keyboard_routine() {
    Byte event = inb(0x60);
    // make/break
    // make: key pressed
    // break: key released
    Byte make = !(event >> 7); 
    Byte code = event & 0x7f;
    if (!make) return;
    
    char c = char_map[code];

    if (c == '\0') {
        c = 'C';
    }
    printc_xy(79, 24, c);
    
    dbg("[keyboard_routine] Unblocking first\n");
    dbg("[keyboard_routine] Setting char_read to '%c'\n", &c);
    char_read = c;
    
    unblock_first(); // unblock first keyboard blocked     
}

//#################//
//### PARCIAL 1 ###//
//#################//

char char_read = 'X';
struct list_head keyboard_blocked;

void block_for_keyboard() {
    struct task_struct* task = current();
    // task->state = ST_BLOCKED; // Only needed if update_process_state_rr does not set it
    update_process_state_rr(task, &keyboard_blocked);
    sched_next_rr();
}

void unblock_first() {
    if (list_empty(&keyboard_blocked)) {
        return;
    }
    struct list_head* head = list_first(&keyboard_blocked);
    list_del(head);
    struct task_struct* task = list_head_to_task_struct(head);
    task->state = ST_RUN;
    list_add(head, &readyqueue); // Add to the first entry on the list
    sched_next_rr();                       // force a task_switch
}