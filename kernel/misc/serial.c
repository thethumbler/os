#include <system.h>
#include <io.h>
#include <serial.h>

static void init_com1()
{
	outb(COM1 + 1, 0x00);	// Disable all interrupts
	outb(COM1 + 3, 0x80);	// Enable DLAB
	outb(COM1 + 0, 0x03);	// Set divisor to 3
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x03);	// 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7);	// Enable FIFO, clear it with 14-byte threshold
	outb(COM1 + 4, 0x0B);	// IRQs enabled
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
	while(!serial_transmit_empty());
	outb(COM1, chr);
}

static void serial_str(uint8_t *str)
{
	while(*str) serial_write(*str++);
}


serial_t serial = (serial_t){
	.init = &init_com1,
	.write = &serial_write,
	.write_str = &serial_str,
};
