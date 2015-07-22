#include <system.h>
#include <io.h>
#include <serial.h>
#include <tty.h>
#include <devfs.h>
#include <console.h>

inode_t serial_tty;
tty_device_t serial_tty_dev;

static uint8_t serial_enabled = 0;

static void init_com1()
{
#if _DBG_CON_
	
	serial_tty_dev = 
		(tty_device_t)
		{
			.id			= 0,
			.master		= NULL,
			.pos		= 0,
			.virtcon	= get_virtcon(&serial_tty_dev, 80, 25),
			.p			= NULL,
			.buf		= NULL,
			.len		= 0,
		};
		
	
	serial_tty = 
		(inode_t)
		{
			.type	= FS_CHRDEV,
			.fs		= &devfs,
			.dev	= &ttydev,
			.p		= &serial_tty_dev,
		};

	cur_tty = &serial_tty;
#else
	outb(COM1 + 1, 0x00);	// Disable all interrupts
	outb(COM1 + 3, 0x80);	// Enable DLAB
	outb(COM1 + 0, 0x03);	// Set divisor to 3
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x03);	// 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7);	// Enable FIFO, clear it with 14-byte threshold
	outb(COM1 + 4, 0x0B);	// IRQs enabled
#endif
	serial_enabled = 1;
}

static uint8_t serial_recieved()
{
	return inb(COM1 + 5) & 1;
}

static uint8_t serial_read()
{
	while(!serial_recieved());
	return inb(COM1);
}

static uint8_t serial_transmit_empty()
{
	return inb(COM1 + 5) & 0x20;
}

static void serial_write(uint8_t chr)
{
	if(serial_enabled)
	{
#if _DBG_CON_
		serial_tty.fs->write(&serial_tty, 0, 1, &chr);
#else
		while(!serial_transmit_empty());
		outb(COM1, chr);
#endif
	}
}

static void serial_str(uint8_t *str)
{
	if(serial_enabled)
	{
#if _DBG_CON_
		serial_tty.fs->write(&serial_tty, 0, strlen(str), str);
#else
		while(*str) serial_write(*str++);
#endif
	}
}

static void serial_end()
{
	serial_enabled = 0;
#if _DBG_CON_
	//kfree(serial_tty_dev.virtcon->buf);
	//kfree(serial_tty_dev.virtcon);
#endif
}

serial_t serial = (serial_t){
	.init = &init_com1,
	.write = &serial_write,
	.write_str = &serial_str,
	.end = &serial_end,
};
