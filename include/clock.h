#ifndef __CLOCK_H__
#define __CLOCK_H__

extern int zeos_ticks;

void clock_routine();
void clock_handler();

void reset_clock_ticks();
int get_clock_ticks();


#endif /* __CLOCK_H__ */