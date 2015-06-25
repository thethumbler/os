#ifndef _PIT_H
#define _PIT_H

void pit_set_freq(uint32_t hz);
void pit_install();

extern uint32_t ticks;
extern uint32_t sub_ticks;
extern uint32_t pit_freq;
extern uint32_t sub_tick_us;

#endif
