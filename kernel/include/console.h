#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <system.h>
#include <tty.h>

typedef struct virtcon_struct virtcon_device_t;

struct virtcon_struct
{
	uint8_t *buf;
	uint32_t pos, h, w;
	
	void (*putc)(virtcon_device_t *virtcon, uint8_t chr, uint32_t x, 
				 uint32_t y, uint32_t fg, uint32_t bg);
	void (*puts)(virtcon_device_t *virtcon, uint8_t *str, uint32_t size,
				 uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
	void (*setcur)(virtcon_device_t *virtcon, uint32_t c, uint32_t r);
	void (*scroll)(virtcon_device_t *virtcon, uint32_t n);
	void (*draw)(virtcon_device_t *virtcon);
	tty_device_t *tty;
};

virtcon_device_t *get_virtcon(tty_device_t *tty, uint32_t w, uint32_t h);

#endif
