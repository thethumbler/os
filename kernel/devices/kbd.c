#include <system.h>
#include <isr.h>
#include <kbd.h>
#include <vfs.h>

uint32_t pos = 0;

void kbd_wait(void) 
{
	while(inb(0x64) & 2);
}

uint32_t spec = 0;

void kbd_handler(regs_t *r) 
{
	kbd_wait();
	uint8_t scancode = inb(0x60);
	if(scancode == 0xE0)
	{
		spec = 1;
		return;
	}
	/*
	static inode_t *i;
	if(!i) i = vfs_trace_path(vfs_root, "/dev/console");
	if(spec)
	{
		switch(scancode)
		{
			case 0x4D: ++pos; break;
			case 0x4B: --pos; break;
		}
		spec = 0;
	}
	
	i->fs->ioctl(i, 1, pos);
	*/
	extern void tty_kbd(uint8_t);
	tty_kbd(scancode);
}

