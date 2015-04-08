#ifndef _SERIAL_H
#define _SERIAL_H

#define COM1 0x3F8

typedef struct {
	void (*init) ();
	void (*write) (uint8_t);
	void (*write_str) (uint8_t*);
} serial_t;

serial_t serial;

#endif
