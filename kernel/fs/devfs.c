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

static uint64_t devfs_read(inode_t *inode, uint64_t offset, uint64_t size, void *buf)
{
	return inode->dev->read(inode, offset, size, buf);
}

static uint64_t devfs_write(inode_t *inode, uint64_t offset, uint64_t size, void *buf)
{
	return inode->dev->write(inode, offset, size, buf);
}

static uint32_t devfs_ioctl(inode_t *inode, uint64_t request, ...)
{
	va_list args;
	va_start(args, request);
	return inode->dev->ioctl(inode, request, args);
	va_end(args);
}

static uint32_t devfs_mount(inode_t *dst, inode_t *src_unused)
{
	vfs_mount(dst, "/", &dev_root);
	return 1;
}

fs_t devfs =
	(fs_t)
	{
		.name = "devfs",
		.read = &devfs_read,
		.write = &devfs_write,
		.ioctl = &devfs_ioctl,
		.mount = &devfs_mount,
	};
