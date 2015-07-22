#ifndef _DEVICE_H
#define _DEVICE_H

#include <system.h>
#include <va_list.h>

typedef enum
{
	DEV_CHR = 1,
	DEV_BLK = 2,
} dev_type;

typedef struct dev_struct dev_t;

#include <vfs.h>

struct dev_struct
{
	uint8_t		*name;
	dev_type	type;
	uint32_t	(*probe)(inode_t *inode);
	uint32_t	(*read) (inode_t *inode, uint64_t offset, uint64_t size, void *buf);
	uint32_t	(*write)(inode_t *inode, uint64_t offset, uint64_t size, void *buf);
	uint32_t	(*ioctl)(inode_t *inode, uint64_t request, va_list args);
//	void 		*p;	// To be used by device handler
} __attribute__((packed));

extern dev_t ramdev;
extern dev_t condev;

typedef struct ramdev_private_struct
{
	void *ptr;
	uint64_t size;
}ramdev_private_t;

typedef struct
{
	void (*init)(void);
} devman_t;

extern devman_t devman;

#endif
