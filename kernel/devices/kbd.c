#include <system.h>
#include <isr.h>
#include <kbd.h>

void kbd_wait(void) 
{
	while(inb(0x64) & 2);
}

void kbd_handler(regs_t *r) 
{
	kbd_wait();
	uint8_t scancode = inb(0x60);
	extern void tty_kbd(uint8_t);
	tty_kbd(scancode);
}

