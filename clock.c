#include "zeos_interrupt.h"

int zeos_ticks;

void clock_routine() {
    zeos_ticks += 1;
    zeos_show_clock();
}