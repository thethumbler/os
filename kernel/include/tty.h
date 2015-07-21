#ifndef _TTY_H
#define _TTY_H

#include <system.h>
#include <process.h>
#include <vfs.h>

typedef struct tty_master_device_struct tty_master_t;
typedef struct tty_device_struct tty_device_t;

#include <console.h>

struct tty_master_device_struct
{
	uint32_t		cur_tty;	//Currently active tty
	
	void (*invoke)(tty_device_t *tty, uint32_t req, ...);
	// Methods for console or terminal emulator
	inode_t *console;
};

struct tty_device_struct
{
	uint32_t		id;
	tty_master_t	*master;
	//uint8_t			*ptr;	// tty buffer
	uint32_t		pos;
	virtcon_device_t *virtcon;
	
	// TODO : add process group id member so every tty can be assigned to one group
	process_t		*p;		// Process currently reading from TTY
	uint8_t			*buf;	// The buffer of the currently reading process
	uint32_t		len;	// Currently reading process buffer length
};

void ttym_invoke(tty_device_t *tty, uint32_t req, ...);
typedef enum
{
	TTYM_WRITE,
	TTYM_SWITCH,
}ttym_requests;

dev_t ttydev;

extern inode_t *cur_tty;

#endif
