#ifndef _DEVICE_H
#define _DEVICE_H

#include <system.h>

typedef enum
{
	DEV_CHAR = 1,
	DEV_BLOCK = 2,
} dev_type;

typedef struct dev_struct dev_t;

struct dev_struct
{
	uint8_t		*name;
	dev_type	type;
	uint32_t	(*read) (dev_t *dev, uint32_t offset, uint32_t size, uint8_t *buf);
	uint32_t	(*write)(dev_t *dev, uint32_t offset, uint32_t size, uint8_t *buf);
	void 		*p;	// To be used by device handler
} __attribute__((packed));

#endif
