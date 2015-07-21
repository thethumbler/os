#include <system.h>
#include <device.h>
#include <vfs.h>

static uint32_t ramdev_read(inode_t *inode, uint32_t offset, uint32_t size, void *_buf)
{
	ramdev_private_t *p = inode->p;
	if(offset <= p->size)
	{
		uint32_t to_read = MIN(size, p->size - offset);
		memcpy(_buf, p->ptr, to_read);
		return to_read;
	}
	return -1;
}

static uint32_t ramdev_write(inode_t *inode, uint32_t offset, uint32_t size, void *_buf)
{
	ramdev_private_t *p = inode->p;
	if(offset <= p->size)
	{
		uint32_t to_write = MIN(size, p->size - offset);
		memcpy(p->ptr, _buf, to_write);
		return to_write;
	}
	return -1;
}

dev_t ramdev = 
	{
		.name = "ramdev",
		.type = DEV_CHR,
		.read = &ramdev_read,
		.write = &ramdev_write,
	};
