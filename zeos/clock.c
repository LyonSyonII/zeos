#include "clock.h"
#include "zeos_interrupt.h"

int zeos_ticks;

void reset_clock_ticks() {
    zeos_ticks = 0;
}

int get_clock_ticks() {
    return zeos_ticks;
}

void clock_routine() {
    zeos_ticks++;
    zeos_show_clock();
}