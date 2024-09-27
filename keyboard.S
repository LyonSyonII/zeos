#include <asm.h>
#include <segment.h>
#include <entry.h>

ENTRY(keyboard_handler)
    SAVE_ALL // Save the current context
    EOI
    call keyboard_routine
    RESTORE_ALL
    iret
