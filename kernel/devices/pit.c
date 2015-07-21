#include <system.h>
#include <isr.h>
#include <scheduler.h>
#include <pit.h>
#include <vfs.h>

uint32_t ticks = 0;
uint32_t sub_ticks = 0;
uint32_t pit_freq = 0;
uint32_t sub_tick_us = 0;

void pit_set_freq(uint32_t hz)
{
	pit_freq = hz;
	sub_tick_us = 1000000/hz;
	
	outb(0x43, 0x36);
	uint32_t div = 1193180/hz;
	outb(0x40, div & 0xFF);
	outb(0x40, (div >> 8) & 0xFF);
}

void pit_irq_handler()
{	
	if(sub_ticks++ == pit_freq)
	{
		ticks++;
		sub_ticks = 0;
	}
	
	if(!(sub_ticks * sub_tick_us % (1000 * 18))) schedule();
}

void pit_install()
{
	irq_install_handler(0, pit_irq_handler);
}

