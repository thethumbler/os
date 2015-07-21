#include <system.h>
#include <kmem.h>
#include <vfs.h>
#include <initramfs.h>
#include <string.h>

static inode_t *cpiofs_load(inode_t *inode)
{
	inode_t *rootfs = kmalloc(sizeof(inode_t));
	
	rootfs->name = NULL;
	rootfs->type = FS_DIR;
	rootfs->parent = NULL;
	rootfs->fs = &initramfs;
	rootfs->list = kmalloc(sizeof(dentry_t));
	dentry_t *list = rootfs->list;

	list->count = 0;
	
	dentry_t *tmp;
	
	cpio_hdr_t *cpio = ((ramdev_private_t*)inode->p)->ptr;
	
	while(*(uint8_t*)cpio)
	{
		uint32_t size = cpio->filesize[0] * 0x10000 + cpio->filesize[1];
		uint8_t *name = (uint8_t*)cpio + sizeof(*cpio);
		if(!strcmp(name, "TRAILER!!!")) break;
		if(!strcmp(name, ".")) goto next;
		uint8_t *filename; uint32_t i;
		for( i = cpio->namesize - 1; i && (name[i] != '/'); --i);
		filename = name + (i?++i:i);
		uint8_t *path = strdup(name);
		path[i] = '\0';
		inode_type type = ((cpio->mode & 0170000 ) == 0040000)?FS_DIR:FS_FILE;

		void *data = name + cpio->namesize + cpio->namesize%2;
		inode_t *new_node = kmalloc(sizeof(inode_t));
		*new_node = 
			(inode_t)
			{
				.name = strdup(filename),
				.size = size,
				.type = type,
				.fs = &initramfs,
				.p = data,
			};
			
		vfs_create(rootfs, *path ? path : (uint8_t*)"/", new_node);
		kfree(path);
		
		next:
		cpio = (typeof(cpio))
			(name + cpio->namesize + (cpio->namesize%2) + size + (size%2));
	}

	return rootfs;
}

static uint32_t cpiofs_read(inode_t *inode, uint32_t offset, uint32_t len, void *buf_p)
{
	uint8_t *buf = (uint8_t*)buf_p;
	if(offset > inode->size) return 0;
	uint32_t size = MIN(len, inode->size - offset), _size = size;
	uint8_t *_buf = (uint8_t*)inode->p + offset;
	while(size--)
		*buf++ = *_buf++;

	return _size;
}

fs_t initramfs = 
	(fs_t)
	{
		.name = "initramfs",
		.load = cpiofs_load,
		.read = cpiofs_read,
		.write = NULL,
	};
