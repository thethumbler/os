#ifndef _TTY_H
#define _TTY_H

#include <system.h>
#include <process.h>

typedef struct tty_device_struct
{
	uint8_t is_master;
	uint32_t id;
	void *master;
	uint8_t *ptr;
	uint32_t pos;
	uint32_t row;
	uint32_t col;
	process_t *p;	// Process currently reading from TTY
	uint8_t *buf;	// The buffer of the currently reading process
	uint32_t len;	// Currently reading process buffer length
}tty_device_t;

fs_t devtty;

#endif
