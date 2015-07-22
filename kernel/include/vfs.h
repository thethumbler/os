#ifndef _VFS_H
#define _VFS_H

#include <system.h>

typedef struct filesystem_struct fs_t;
typedef enum { FS_FILE, FS_DIR, FS_CHRDEV, FS_BLKDEV, FS_SYMLINK, FS_PIPE, FS_MOUNTPOINT } inode_type; 
typedef struct inode_struct inode_t;
typedef struct dentry_struct dentry_t;

typedef struct filesystem_struct
{
	uint8_t		*name;
	inode_t*	(*load)  (inode_t*);
	uint32_t	(*link)  (inode_t*, uint8_t*);
	uint32_t	(*unlink)(inode_t*, uint8_t*);
	uint64_t	(*read)  (inode_t*, uint64_t, uint64_t, void*);
	uint64_t	(*write) (inode_t*, uint64_t, uint64_t, void*);
	uint32_t	(*ioctl) (inode_t*, uint64_t, ...);
	uint32_t	(*mount) (inode_t*, inode_t*);
}fs_t;

#include <device.h>

struct inode_struct
{
	uint8_t		*name;
	uint32_t	size;
	inode_type	type;
	inode_t 	*parent;
	dentry_t	*list;
	fs_t		*fs;
	dev_t		*dev;
	void		*p;	// To be used by filesystem handler
	
	inode_t		*next;	// For directories
};

struct dentry_struct
{
	uint32_t count;
	inode_t *head;
};

typedef struct vfs_mountpoint_struct
{
	inode_t *inode;
	inode_t *old_inode;
}vfs_mountpoint_t;

typedef struct dirent {
	uint32_t d_ino;
	char d_name[256];
} dirent;

typedef struct
{
	void   (*init) ();
	fs_t * (*getfs)(uint8_t *name);
} fsman_t;

extern fsman_t fsman;

extern fs_t vfs;

extern inode_t *vfs_root;
inode_t *vfs_trace_path(inode_t*, uint8_t*);
inode_t *vfs_create(inode_t*, uint8_t*, inode_t*);
inode_t *vfs_mount(inode_t*, uint8_t*, inode_t*);
void vfs_tree(inode_t*);

uint64_t vfs_read (inode_t*, uint64_t, uint64_t, void*);
uint64_t vfs_write(inode_t*, uint64_t, uint64_t, void*);

#endif
