#include <system.h>
#include <kmem.h>
#include <debug.h>
#include <vfs.h>
#include <string.h>

inode_t *vfs_root;

void vfs_mount_root(inode_t *node)
{
	vfs_root = kmalloc(sizeof(inode_t));
	vfs_mount(vfs_root, "/", node);
}

inode_t *vfs_trace_path(inode_t *inode, uint8_t *_path)
{
	if(!_path || (_path && !*_path)) return inode;
	uint8_t *path = strdup(_path);
	uint32_t len = strlen(path);
	uint8_t *end_path = path + len;
	uint32_t i;
	for( i = 0; i < len; ++i)
		if(path[i] == '/')
			path[i] = '\0';
			
	if((!*path) && len == 1) return inode;
	if(!*path) ++path;

	inode_t *tmp = inode;
	
	while(tmp)
	{
		if(tmp->type == FS_MOUNTPOINT)
			tmp = ((vfs_mountpoint_t*)tmp->p)->inode;
		
		if(!tmp->list) return NULL;
		if(!tmp->list->count) return NULL;
		uint32_t k = 0;
		inode_t *_tmp = tmp->list->head;
		if(!_tmp) return NULL;
		while(_tmp && k < tmp->list->count)
		{
			if(!strcmp(path, _tmp->name)) break;
			_tmp = _tmp->next;
			++k;
		}
		if( !_tmp || !_tmp->name || strcmp(path, _tmp->name) ) return NULL;
		tmp = _tmp;
		while(path < end_path && *path++);
		if(path >= end_path) return tmp;
	}
}

inode_t *vfs_create(inode_t *root, uint8_t *path, inode_t *new_node)
{
	inode_t *dir = vfs_trace_path(root, path);
	
	if(!dir) return NULL;
	if(!dir->list)
	{
		dir->list = kmalloc(sizeof(dentry_t));
		dir->list->count = 0;
	}
	uint32_t k;
	
	if(!dir->list->head) dir->list->head = kmalloc(sizeof(inode_t));
	
	++dir->list->count;
	new_node->next = dir->list->head;
	new_node->parent = dir;
	dir->list->head = new_node;
	return new_node;
}

inode_t *vfs_mount(inode_t *root, uint8_t *path, inode_t *new_node)
{
	inode_t *dir = vfs_trace_path(root, path);
	if(!dir) return NULL;
	
	inode_t *tmp = kmalloc(sizeof(inode_t));
	memcpy(tmp, dir, sizeof(inode_t));
	
	dir->type = FS_MOUNTPOINT;

	vfs_mountpoint_t *mount = kmalloc(sizeof(vfs_mountpoint_t));
	mount->inode = new_node;
	mount->old_inode = tmp;
	
	dir->p    = mount;
	//dir->fs   = &vfs;
	
	return dir;
}

void vfs_tree(inode_t *node)
{
	static uint32_t level = 0;
	if(!node) return;
	if(node->name) debug("%s", node->name);
	if(node->type == FS_DIR || node->type == FS_MOUNTPOINT) 
	{
		debug("/");
		if(node->type == FS_MOUNTPOINT)
		{
			vfs_mountpoint_t *m = node->p;
			vfs_tree(m->inode);
			return;
		}
		if(node->list)
		{
			debug("\n");
			++level;
			uint32_t i, j;
			for(j = 0; j < node->list->count; ++j)
			{
				if(level > 1)
					for(i = 1; i < 2 * level - 2; ++i) debug(" ");
				debug("→ ");
				
				inode_t *tmp = node->list->head;
				uint32_t k = 0;
				while(k++ < j) tmp = tmp->next;
				vfs_tree(tmp);
				debug("\n");
			}
			--level;
			return;
		}
	}
}

uint32_t vfs_read(inode_t *inode, uint64_t offset, uint64_t len, void *buf)
{
	return inode->fs->read(inode, offset, len, buf);
}

uint32_t vfs_write(inode_t *inode, uint64_t offset, uint64_t len, void *buf)
{
	return inode->fs->write(inode, offset, len, buf);
}
