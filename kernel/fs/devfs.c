#include <system.h>
#include <vfs.h>
#include <devfs.h>

inode_t dev_root = 
(inode_t){
	.name	= NULL,
	.size	= 0,
	.type	= FS_DIR,
	.parent	= NULL,
	.list	= NULL,
	.fs		= &devfs,
	.dev	= &ramdev,
	.p		= NULL,
	.next	= NULL,
};

uint32_t devfs_read(inode_t *inode, uint32_t offset, uint32_t size, void *buf)
{
	return inode->dev->read(inode, offset, size, buf);
}

uint32_t devfs_write(inode_t *inode, uint32_t offset, uint32_t size, void *buf)
{
	return inode->dev->write(inode, offset, size, buf);
}

uint32_t devfs_ioctl(inode_t *inode, uint32_t request, ...)
{
	va_list args;
	va_start(args, request);
	return inode->dev->ioctl(inode, request, args);
	va_end(args);
}

fs_t devfs =
	(fs_t)
	{
		.name = "devfs",
		.read = &devfs_read,
		.write = &devfs_write,
		.ioctl = &devfs_ioctl,
	};
