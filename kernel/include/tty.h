#ifndef _TTY_H
#define _TTY_H

#include <system.h>

typedef struct tty_device_struct
{
	uint8_t is_master;
	uint32_t id;
	void *master;
	uint8_t *ptr;
	uint32_t pos;
	uint32_t row;
	uint32_t col;
}tty_device_t;

fs_t devtty;

#endif
